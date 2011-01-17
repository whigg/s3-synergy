/*
 * File:   SynL2Writer.cpp
 * Author: thomass
 *
 * Created on November 17, 2010, 3:40 PM
 */

#include <iostream>

#include "../util/IOUtils.h"
#include "../util/NetCDF.h"
#include "SynL2Writer.h"

using std::make_pair;

SynL2Writer::SynL2Writer() : Writer("SYN_L2_WRITER") {
}

SynL2Writer::~SynL2Writer() {
}

void SynL2Writer::process(Context& context) {
    Dictionary& dict = *context.getDictionary();
    const vector<SegmentDescriptor*> segmentDescriptors =
            dict.getProductDescriptor(Constants::SYMBOLIC_NAME_L2).getSegmentDescriptors();

    foreach(SegmentDescriptor* segmentDescriptor, segmentDescriptors) {
        const Segment& segment = context.getSegment(segmentDescriptor->getName());
        const Grid& grid = segment.getGrid();
        const vector<VariableDescriptor*> variableDescriptors =
                segmentDescriptor->getVariableDescriptors();
        const size_t startL = getStartL(context, segment);
        const size_t endL = context.getMaxLWritable(segment, *this);

        foreach(VariableDescriptor* variableDescriptor, variableDescriptors) {
            const string varName = variableDescriptor->getName();

            const string ncFileName = variableDescriptor->getNcFileName();

            if (!exists(varIdMap, varName)) {
                throw logic_error("Unknown variable '" + varName + "'.");
            }
            if (!exists(fileIdMap, ncFileName)) {
                throw logic_error("Unknown netCDF file '" + ncFileName + "'.");
            }
            if (!exists(dimIdMap, ncFileName)) {
                throw logic_error("Unknown netCDF file '" + ncFileName + "'.");
            }
            const int varId = varIdMap[varName];
            const int ncId = fileIdMap[ncFileName];
            const valarray<int>& dimIds = dimIdMap[ncFileName];

            valarray<size_t> starts = IOUtils::createStartVector(dimIds.size(), startL);
            valarray<size_t> sizes = IOUtils::createCountVector(dimIds.size(), grid.getSizeK(), endL - startL + 1, grid.getSizeM());

            if (context.getLogging() != 0) {
                context.getLogging()->progress("Writing variable " + varName + " of segment [" + segment.toString() + "]", getId());
            }
            const Accessor& accessor = segment.getAccessor(varName);
            NetCDF::putData(ncId, varId, starts, sizes, accessor.getUntypedData());
        }
        context.setMaxLComputed(segment, *this, endL);
    }
}

void SynL2Writer::start(Context& context) {
    // todo: extract directory path from job order ...
    Dictionary& dict = *context.getDictionary();

    const vector<SegmentDescriptor*> segmentDescriptors =
            dict.getProductDescriptor(Constants::SYMBOLIC_NAME_L2).getSegmentDescriptors();

    foreach(SegmentDescriptor* segmentDescriptor, segmentDescriptors) {
        const Segment& segment = context.getSegment(segmentDescriptor->getName());
        const vector<VariableDescriptor*> variableDescriptors =
                segmentDescriptor->getVariableDescriptors();

        foreach(VariableDescriptor* variableDescriptor, variableDescriptors) {
            createNcVar(*variableDescriptor, segment.getGrid());
        }
    }
    // TODO - global attributes ...

    pair<string, int> fileIdPair;

    foreach(fileIdPair, fileIdMap) {
        NetCDF::setDefinitionPhaseFinished(fileIdPair.second);
    }
}

void SynL2Writer::stop(Context& context) {
    pair<string, int> fileIdPair;

    foreach(fileIdPair, fileIdMap) {
        NetCDF::closeFile(fileIdPair.second);
    }

    fileIdMap.clear();
    dimIdMap.clear();
    varIdMap.clear();
}

void SynL2Writer::createNcVar(const VariableDescriptor& variableDescriptor, const Grid& grid) {
    const string ncFileName = variableDescriptor.getNcFileName();
    const string varName = variableDescriptor.getName();

    if (exists(varIdMap, varName)) {
        throw logic_error("Variable '" + varName + "' already exists.");
    }
    if (!exists(fileIdMap, ncFileName)) {
        int fileId = NetCDF::createFile(variableDescriptor.getNcFileName().append(".nc").c_str());

        const size_t sizeK = grid.getSizeK();
        const size_t sizeL = grid.getMaxL() - grid.getMinL() + 1;
        const size_t sizeM = grid.getSizeM();
        vector<Dimension*> dims = variableDescriptor.getDimensions();
        size_t dimCount = 0;
        if (grid.getSizeK() > 1) {
            dimCount++;
        }
        if (grid.getSizeL() > 1) {
            dimCount++;
        }
        if (grid.getSizeM() > 1) {
            dimCount++;
        }
        if (dimCount != dims.size()) {
            throw logic_error("dimCount != dims.size()");
        }
        valarray<int> dimIds(dimCount);
        int dimIndex = 0;
        if (sizeK > 1) {
            dimIds[dimIndex] = NetCDF::defineDimension(fileId, dims[dimIndex]->getName(), sizeK);
            dimIndex++;
        }
        if (grid.getSizeL() > 1) {
            dimIds[dimIndex] = NetCDF::defineDimension(fileId, dims[dimIndex]->getName(), sizeL);
            dimIndex++;
        }
        if (sizeM > 1) {
            dimIds[dimIndex] = NetCDF::defineDimension(fileId, dims[dimIndex]->getName(), sizeM);
        }

        fileIdMap[ncFileName] = fileId;
        dimIdMap.insert(make_pair(ncFileName, dimIds));
    }
    const int fileId = fileIdMap[ncFileName];
    const valarray<int>& dimIds = dimIdMap[ncFileName];
    int varId = NetCDF::defineVariable(fileId, variableDescriptor.getNcVarName().c_str(), variableDescriptor.getType(), dimIds);
    varIdMap[varName] = varId;

    foreach(Attribute* attribute, variableDescriptor.getAttributes()) {
        NetCDF::addAttribute(fileId, varId, *attribute);
    }
}
