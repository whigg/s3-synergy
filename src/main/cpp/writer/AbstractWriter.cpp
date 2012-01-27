
/*
 * File:   AbstractWriter.cpp
 * Author: thomass
 *
 * Created on Jan 11, 2012, 16:29
 */

#include <fstream>
#include <stdexcept>

#include "../core/Config.h"
#include "../util/IOUtils.h"
#include "../util/NetCDF.h"

#include "AbstractWriter.h"

using std::runtime_error;

AbstractWriter::AbstractWriter(const string& name) :
        BasicModule(name) {
}

AbstractWriter::~AbstractWriter() {
	pair<string, int> fileIdPair;
	foreach(fileIdPair, ncFileIdMap) {
	    try {
	        NetCDF::closeFile(fileIdPair.second);
	    } catch (exception& ignored) {
	    }
	}
}

void AbstractWriter::start(Context& context) {
	targetDirPath = path(context.getJobOrder().getIpfProcessors().at(0).getOutputList().at(0).getFileName());
	if (!targetDirPath.has_root_directory()) {
		targetDirPath = Constants::S3_SYNERGY_HOME / targetDirPath;
	}
	context.getLogging().info("target product path is '" + targetDirPath.string() + "'", getId());

	const Dictionary& dict = context.getDictionary();
	const ProductDescriptor& productDescriptor = dict.getProductDescriptor(getProductDescriptorIdentifier());
	const vector<SegmentDescriptor*> segmentDescriptors = getSegmentDescriptors(dict);

	foreach(SegmentDescriptor* segmentDescriptor, segmentDescriptors) {
	    const string segmentName = segmentDescriptor->getName();
	    if (context.hasSegment(segmentName)) {
	        const Segment& segment = context.getSegment(segmentName);
	        const vector<VariableDescriptor*> variableDescriptors = segmentDescriptor->getVariableDescriptors();
	        foreach(VariableDescriptor* variableDescriptor, variableDescriptors) {
	            context.getLogging().info("Defining variable for " + variableDescriptor->toString(), getId());
	            defineNcVar(context, productDescriptor, *segmentDescriptor, *variableDescriptor, segment.getGrid());
	        }
	    }
	}

	pair<string, int> fileIdPair;
	foreach(fileIdPair, ncFileIdMap) {
	    NetCDF::terminateFile(fileIdPair.second);
	}
}

void AbstractWriter::process(Context& context) {
    const vector<SegmentDescriptor*> segmentDescriptors = getSegmentDescriptors(context.getDictionary());

    valarray<size_t> origin;
    valarray<size_t> shape;
    foreach(const SegmentDescriptor* segmentDescriptor, segmentDescriptors) {
        const string segmentName = segmentDescriptor->getName();
        if (context.hasSegment(segmentName)) {
            const Segment& segment = context.getSegment(segmentName);
            const Grid& grid = segment.getGrid();
            const long firstL = segment.getGrid().getFirstL();
            context.getLogging().debug("Segment [" + segment.toString() + "]: firstL = " + lexical_cast<string>(firstL), getId());
            const long lastL = segment.getGrid().getLastL();
            context.getLogging().debug("Segment [" + segment.toString() + "]: lastL = " + lexical_cast<string>(lastL), getId());

            if (firstL <= lastL) {
                const vector<VariableDescriptor*> variableDescriptors = segmentDescriptor->getVariableDescriptors();
                foreach(const VariableDescriptor* variableDescriptor, variableDescriptors) {
                    const string varName = variableDescriptor->getName();
                    if (segment.hasVariable(varName)) {
                        const string ncFileBasename = variableDescriptor->getNcFileBasename();
                        if (!contains(ncVarIdMap, varName)) {
                            BOOST_THROW_EXCEPTION( logic_error("Unknown variable '" + varName + "'."));
                        }
                        if (!contains(ncFileIdMap, ncFileBasename)) {
                            BOOST_THROW_EXCEPTION( logic_error("Unknown netCDF file '" + ncFileBasename + "'."));
                        }
                        if (!contains(ncDimIdMap, ncFileBasename)) {
                            BOOST_THROW_EXCEPTION( logic_error("Unknown netCDF file '" + ncFileBasename + "'."));
                        }
                        const int varId = ncVarIdMap[varName];
                        const int ncId = ncFileIdMap[ncFileBasename];
                        const valarray<int>& dimIds = ncDimIdMap[ncFileBasename];
                        context.getLogging().progress("Writing variable '" + varName + "' of segment [" + segment.toString() + "]", getId());
                        const Accessor& accessor = segment.getAccessor(varName);
                        if (accessor.canReturnDataPointer()) {
                            IOUtils::createStartVector(dimIds.size(), firstL, origin);
                            IOUtils::createCountVector(dimIds.size(), grid.getSizeK(), lastL - firstL + 1, grid.getSizeM(), shape);
                        	NetCDF::putData(ncId, varId, origin, shape, accessor.getUntypedData());
                        } else {
                            putData(ncId, varId, dimIds.size(), accessor, firstL, lastL, grid);
                        }
                    }
                }
                context.setLastComputedL(segment, *this, lastL);
            }
        }
    }
    writeCommonVariables(context);
}

void AbstractWriter::stop(Context& context) {
	createSafeProduct(context);

	ncVarIdMap.clear();
	ncDimIdMap.clear();
	ncFileIdMap.clear();
}

void AbstractWriter::defineNcVar(const Context& context, const ProductDescriptor& productDescriptor, const SegmentDescriptor& segmentDescriptor, const VariableDescriptor& variableDescriptor, const Grid& grid) {
	const string ncFileBasename = variableDescriptor.getNcFileBasename();

	const bool fileForVariableExists = contains(ncFileIdMap, ncFileBasename);
	if (!fileForVariableExists) {
		if (!boost::filesystem::exists(targetDirPath)) {
			if (!boost::filesystem::create_directories(targetDirPath)) {
				BOOST_THROW_EXCEPTION( runtime_error("Cannot create directory '" + targetDirPath.string() + "'."));
			}
		}
		const path ncFilePath = targetDirPath / (ncFileBasename + ".nc");
		const int fileId = NetCDF::createFile(ncFilePath.string());

		putGlobalAttributes(fileId, variableDescriptor, productDescriptor.getAttributes());

		const string& variableName = variableDescriptor.getName();

		valarray<int> dimIds;
		defineDimensions(fileId, variableName, variableDescriptor.getDimensions(), grid, dimIds);
		map<const VariableDescriptor*, valarray<int> > commonDimIds;
		defineCommonDimensions(fileId, segmentDescriptor.getName(), context.getDictionary(), commonDimIds);
		defineCommonVariables(fileId, segmentDescriptor.getName(), context.getDictionary(), commonDimIds);

		ncFileIdMap[ncFileBasename] = fileId;
		ncDimIdMap.insert(make_pair(ncFileBasename, dimIds));
	}
	const int fileId = ncFileIdMap[ncFileBasename];
	resolveSubsampling(fileId, segmentDescriptor.getName());
	const valarray<int>& dimIds = ncDimIdMap[ncFileBasename];
	const int varId = NetCDF::defineVariable(fileId, variableDescriptor.getNcVarName(), variableDescriptor.getType(), dimIds);
	ncVarIdMap[variableDescriptor.getName()] = varId;

	foreach(const Attribute* attribute, variableDescriptor.getAttributes()) {
	    NetCDF::putAttribute(fileId, varId, *attribute);
	}
}

void AbstractWriter::defineDimensions(const int fileId, const string& name, const vector<Dimension*>& dimensions, const Grid& grid, valarray<int>& dimIds) {
    const long sizeK = grid.getSizeK();
    const long sizeL = grid.getMaxL() - grid.getMinL() + 1;
    const long sizeM = grid.getSizeM();
    const size_t dimCount = dimensions.size();

    dimIds.resize(dimCount, 0);
    switch (dimCount) {
    case 3:
        dimIds[0] = NetCDF::defineDimension(fileId, dimensions[0]->getName(), sizeK);
        dimIds[1] = NetCDF::defineDimension(fileId, dimensions[1]->getName(), sizeL);
        dimIds[2] = NetCDF::defineDimension(fileId, dimensions[2]->getName(), sizeM);
        break;
    case 2:
        dimIds[0] = NetCDF::defineDimension(fileId, dimensions[0]->getName(), sizeL);
        dimIds[1] = NetCDF::defineDimension(fileId, dimensions[1]->getName(), sizeM);
        break;
    case 1:
        dimIds[0] = NetCDF::defineDimension(fileId, dimensions[0]->getName(), sizeM);
        break;
    default:
        BOOST_THROW_EXCEPTION(logic_error("AbstractWriter::createNcVar(): invalid number of dimensions (" + name + ")"));
        break;
    }
}

void AbstractWriter::putGlobalAttributes(int fileId, const VariableDescriptor& variableDescriptor, const vector<Attribute*>& attributes) const {
	foreach (Attribute* attribute, attributes)
			{
				const string& attributeName = attribute->getName();
				if (attributeName.compare("title") == 0) {
					string title;
					if (variableDescriptor.hasAttribute("long_name")) {
						title = variableDescriptor.getAttribute("long_name").getValue();
					} else {
						title = variableDescriptor.getName();
					}
					attribute->setValue(title);
				} else if (attributeName.compare("comment") == 0) {
					string comment = "This dataset contains the '";
					if (variableDescriptor.hasAttribute("long_name")) {
						comment.append(variableDescriptor.getAttribute("long_name").getValue().c_str());
					} else {
						comment.append(variableDescriptor.getName().c_str());
					}
					comment.append(" 'variable.");
					attribute->setValue(comment);
				} else if (attributeName.compare("processor_version") == 0) {
					attribute->setValue(Constants::PROCESSOR_VERSION);
				} else if (attributeName.compare("dataset_name") == 0) {
					attribute->setValue(variableDescriptor.getNcFileBasename());
				} else if (attributeName.compare("dataset_version") == 0) {
					attribute->setValue(Constants::DATASET_VERSION);
				} else if (attributeName.compare("package_name") == 0) {
					attribute->setValue(targetDirPath.filename());
				} else if (attributeName.compare("creation_time") == 0) {
					time_t currentTime;
					time(&currentTime);
					struct tm* currentTimeStructure = gmtime(&currentTime);
					char buffer[80];
					strftime(buffer, 80, "UTC=%Y-%m-%dT%H:%M:%S.000000", currentTimeStructure);
					attribute->setValue(string(buffer));
				}
				NetCDF::putGlobalAttribute(fileId, *attribute);
			}
}

void AbstractWriter::putData(int ncId, int varId, size_t dimCount, const Accessor& accessor, long firstL, long lastL, const Grid& grid) const {
    switch (accessor.getType()) {
    case Constants::TYPE_BYTE:
        putByteData(ncId, varId, dimCount, accessor, firstL, lastL, grid);
        break;
    case Constants::TYPE_SHORT:
        putShortData(ncId, varId, dimCount, accessor, firstL, lastL, grid);
        break;
    case Constants::TYPE_INT:
        putIntData(ncId, varId, dimCount, accessor, firstL, lastL, grid);
        break;
    case Constants::TYPE_LONG:
        putLongData(ncId, varId, dimCount, accessor, firstL, lastL, grid);
        break;
    case Constants::TYPE_UBYTE:
        putUByteData(ncId, varId, dimCount, accessor, firstL, lastL, grid);
        break;
    case Constants::TYPE_USHORT:
        putUShortData(ncId, varId, dimCount, accessor, firstL, lastL, grid);
        break;
    case Constants::TYPE_UINT:
        putUIntData(ncId, varId, dimCount, accessor, firstL, lastL, grid);
        break;
    case Constants::TYPE_ULONG:
        putULongData(ncId, varId, dimCount, accessor, firstL, lastL, grid);
        break;
    case Constants::TYPE_FLOAT:
        putFloatData(ncId, varId, dimCount, accessor, firstL, lastL, grid);
        break;
    case Constants::TYPE_DOUBLE:
        putDoubleData(ncId, varId, dimCount, accessor, firstL, lastL, grid);
        break;
    default:
        BOOST_THROW_EXCEPTION(runtime_error("Unsupported variable type."));
        break;
    }
}

void AbstractWriter::putByteData(int ncId, int varId, size_t dimCount, const Accessor& accessor, long firstL, long lastL, const Grid& grid) const {
    valarray<int8_t> data(grid.getSizeM());
    valarray<size_t> origin;
    valarray<size_t> shape;
    IOUtils::createCountVector(dimCount, 1, 1, grid.getSizeM(), shape);
    for (long l = firstL; l <= lastL; l++) {
        IOUtils::createStartVector(dimCount, l, origin);
        for (long m = 0; m < grid.getSizeM(); m++) {
            data[m] = accessor.getByte(grid.getIndex(0, l, m));
        }
        NetCDF::putData(ncId, varId, origin, shape, &data[0]);
    }
}

void AbstractWriter::putUByteData(int ncId, int varId, size_t dimCount, const Accessor& accessor, long firstL, long lastL, const Grid& grid) const {
    valarray<uint8_t> data(grid.getSizeM());
    valarray<size_t> origin;
    valarray<size_t> shape;
    IOUtils::createCountVector(dimCount, 1, 1, grid.getSizeM(), shape);
    for (long l = firstL; l <= lastL; l++) {
        IOUtils::createStartVector(dimCount, l, origin);
        for (long m = 0; m < grid.getSizeM(); m++) {
            data[m] = accessor.getUByte(grid.getIndex(0, l, m));
        }
        NetCDF::putData(ncId, varId, origin, shape, &data[0]);
    }
}

void AbstractWriter::putShortData(int ncId, int varId, size_t dimCount, const Accessor& accessor, long firstL, long lastL, const Grid& grid) const {
    valarray<int16_t> data(grid.getSizeM());
    valarray<size_t> origin;
    valarray<size_t> shape;
    IOUtils::createCountVector(dimCount, 1, 1, grid.getSizeM(), shape);
    for (long l = firstL; l <= lastL; l++) {
        IOUtils::createStartVector(dimCount, l, origin);
        for (long m = 0; m < grid.getSizeM(); m++) {
            data[m] = accessor.getShort(grid.getIndex(0, l, m));
        }
        NetCDF::putData(ncId, varId, origin, shape, &data[0]);
    }
}

void AbstractWriter::putUShortData(int ncId, int varId, size_t dimCount, const Accessor& accessor, long firstL, long lastL, const Grid& grid) const {
    valarray<uint16_t> data(grid.getSizeM());
    valarray<size_t> origin;
    valarray<size_t> shape;
    IOUtils::createCountVector(dimCount, 1, 1, grid.getSizeM(), shape);
    for (long l = firstL; l <= lastL; l++) {
        IOUtils::createStartVector(dimCount, l, origin);
        for (long m = 0; m < grid.getSizeM(); m++) {
            data[m] = accessor.getUShort(grid.getIndex(0, l, m));
        }
        NetCDF::putData(ncId, varId, origin, shape, &data[0]);
    }
}

void AbstractWriter::putIntData(int ncId, int varId, size_t dimCount, const Accessor& accessor, long firstL, long lastL, const Grid& grid) const {
    valarray<int32_t> data(grid.getSizeM());
    valarray<size_t> origin;
    valarray<size_t> shape;
    IOUtils::createCountVector(dimCount, 1, 1, grid.getSizeM(), shape);
    for (long l = firstL; l <= lastL; l++) {
        IOUtils::createStartVector(dimCount, l, origin);
        for (long m = 0; m < grid.getSizeM(); m++) {
            data[m] = accessor.getInt(grid.getIndex(0, l, m));
        }
        NetCDF::putData(ncId, varId, origin, shape, &data[0]);
    }
}

void AbstractWriter::putUIntData(int ncId, int varId, size_t dimCount, const Accessor& accessor, long firstL, long lastL, const Grid& grid) const {
    valarray<uint32_t> data(grid.getSizeM());
    valarray<size_t> origin;
    valarray<size_t> shape;
    IOUtils::createCountVector(dimCount, 1, 1, grid.getSizeM(), shape);
    for (long l = firstL; l <= lastL; l++) {
        IOUtils::createStartVector(dimCount, l, origin);
        for (long m = 0; m < grid.getSizeM(); m++) {
            data[m] = accessor.getUInt(grid.getIndex(0, l, m));
        }
        NetCDF::putData(ncId, varId, origin, shape, &data[0]);
    }
}

void AbstractWriter::putLongData(int ncId, int varId, size_t dimCount, const Accessor& accessor, long firstL, long lastL, const Grid& grid) const {
    valarray<int64_t> data(grid.getSizeM());
    valarray<size_t> origin;
    valarray<size_t> shape;
    IOUtils::createCountVector(dimCount, 1, 1, grid.getSizeM(), shape);
    for (long l = firstL; l <= lastL; l++) {
        IOUtils::createStartVector(dimCount, l, origin);
        for (long m = 0; m < grid.getSizeM(); m++) {
            data[m] = accessor.getLong(grid.getIndex(0, l, m));
        }
        NetCDF::putData(ncId, varId, origin, shape, &data[0]);
    }
}

void AbstractWriter::putULongData(int ncId, int varId, size_t dimCount, const Accessor& accessor, long firstL, long lastL, const Grid& grid) const {
    valarray<uint64_t> data(grid.getSizeM());
    valarray<size_t> origin;
    valarray<size_t> shape;
    IOUtils::createCountVector(dimCount, 1, 1, grid.getSizeM(), shape);
    for (long l = firstL; l <= lastL; l++) {
        IOUtils::createStartVector(dimCount, l, origin);
        for (long m = 0; m < grid.getSizeM(); m++) {
            data[m] = accessor.getULong(grid.getIndex(0, l, m));
        }
        NetCDF::putData(ncId, varId, origin, shape, &data[0]);
    }
}

void AbstractWriter::putFloatData(int ncId, int varId, size_t dimCount, const Accessor& accessor, long firstL, long lastL, const Grid& grid) const {
    valarray<float> data(grid.getSizeM());
    valarray<size_t> origin;
    valarray<size_t> shape;
    IOUtils::createCountVector(dimCount, 1, 1, grid.getSizeM(), shape);
    for (long l = firstL; l <= lastL; l++) {
        IOUtils::createStartVector(dimCount, l, origin);
        for (long m = 0; m < grid.getSizeM(); m++) {
            data[m] = accessor.getFloat(grid.getIndex(0, l, m));
        }
        NetCDF::putData(ncId, varId, origin, shape, &data[0]);
    }
}

void AbstractWriter::putDoubleData(int ncId, int varId, size_t dimCount, const Accessor& accessor, long firstL, long lastL, const Grid& grid) const {
    valarray<double> data(grid.getSizeM());
    valarray<size_t> origin;
    valarray<size_t> shape;
    IOUtils::createCountVector(dimCount, 1, 1, grid.getSizeM(), shape);
    for (long l = firstL; l <= lastL; l++) {
        IOUtils::createStartVector(dimCount, l, origin);
        for (long m = 0; m < grid.getSizeM(); m++) {
            data[m] = accessor.getDouble(grid.getIndex(0, l, m));
        }
        NetCDF::putData(ncId, varId, origin, shape, &data[0]);
    }
}

void AbstractWriter::createSafeProduct(const Context& context) {
    copyTemplateFiles();
    string manifest = readManifest();
    setStartTime(context, manifest);
    setChecksums(manifest);
    writeManifest(manifest);
    removeManifestTemplate();
}

void AbstractWriter::copyTemplateFiles() const {
    const string sourceDirPath = Constants::S3_SYNERGY_HOME + "/src/main/resources/SAFE_metacomponents/" + getProductDescriptorIdentifier();
    const vector<string> fileNames = IOUtils::getFileNames(sourceDirPath);
    foreach(string fileName, fileNames) {
        boost::filesystem::copy_file(sourceDirPath + "/" + fileName, targetDirPath / fileName);
    }
    boost::filesystem::create_directory(path(targetDirPath.string() + "/schema"));
    const vector<string> schemaFileNames = IOUtils::getFileNames(sourceDirPath + "/schema");
    foreach(string fileName, schemaFileNames) {
        boost::filesystem::copy_file(sourceDirPath + "/schema/" + fileName, targetDirPath / "schema" / fileName);
    }
}

string AbstractWriter::readManifest() const {
    std::ifstream ifs(string(targetDirPath.string() + "/" + getSafeManifestName() + ".template").c_str(), std::ifstream::in);
    std::ostringstream oss;
    char c;
    while (ifs.get(c)) {
        oss.put(c);
    }
    ifs.close();
    return oss.str();
}

void AbstractWriter::setStartTime(const Context& context, string& manifest) const {
    struct tm* ptm = localtime(&context.getStartTime());
    char buffer[32];
    strftime(buffer, 32, "%Y-%m-%dT%H:%M:%S", ptm);
    string startTime = string(buffer);
    replaceString("\\$\\{processing-start-time\\}", startTime, manifest);
}

void AbstractWriter::setChecksums(string& manifest) const {
    pair<string, int> fileIdPair;
    foreach(fileIdPair, ncFileIdMap) {
        string checksum = getMd5Sum(targetDirPath.string() + "/" + fileIdPair.first + ".nc");
        replaceString("\\s*\\$\\{checksum-" + fileIdPair.first + "\\.nc\\}\\s*", checksum, manifest);
        NetCDF::closeFile(fileIdPair.second);
    }
}

void AbstractWriter::writeManifest(string& manifest) const {
    std::ofstream ofs(string(targetDirPath.string() + "/" + getSafeManifestName() + ".xml").c_str(), std::ofstream::out);
    for(size_t i = 0; i < manifest.size(); i++) {
        ofs.put(manifest[i]);
    }
    ofs.close();
}

void AbstractWriter::removeManifestTemplate() const {
    path manifestTemplate = path(targetDirPath.string() + "/" + getSafeManifestName() + ".template");
    boost::filesystem::remove(manifestTemplate);
}

void AbstractWriter::replaceString(const string& toReplace, const string& replacement, string& input) {
    regex expr(toReplace.c_str());
    input = regex_replace(input, expr, replacement);
}

string AbstractWriter::getMd5Sum(const string& file) {
    FILE* pipe = popen(string(Constants::MD5SUM_EXECUTABLE + " " + file).c_str(), "r");
    if (!pipe || !boost::filesystem::exists(path(file))) {
        BOOST_THROW_EXCEPTION(std::invalid_argument("Could not perform command 'md5sum' on file '" + file + "'."));
    }
    char buffer[128];
    std::string result = "";
    while(!feof(pipe)) {
        if(fgets(buffer, 128, pipe) != NULL)
                result += buffer;
    }
    pclose(pipe);
    replaceString("\\s+.*", "", result);
    return result;
}
