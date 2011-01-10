/*
 * File:   Reader.cpp
 * Author: thomass
 *
 * Created on November 18, 2010, 2:08 PM
 */

#include <algorithm>
#include <boost/algorithm/string/predicate.hpp>
#include <iostream>
#include <netcdf.h>
#include <stddef.h>
#include <stdexcept>
#include <valarray>

#include "JobOrder.h"
#include "Logger.h"
#include "Reader.h"
#include "ReaderConstants.h"
#include "IOUtils.h"

using std::min;
using std::max;

Reader::Reader() : DefaultModule("READ"), stepSize(2000) {
}

Reader::~Reader() {
}

void Reader::process(Context& context) {
    // switch between processor configurations

    Dictionary& dict = *(context.getDictionary());
    string processorId = "SYL2";
    string sourceDir = context.getJobOrder()->getProcessorConfiguration(processorId).getInputList()[0]->getFileNames()[0];
    // TODO - replace by dictionary
    const vector<string>& variablesToRead = ReaderConstants::getVariablesToRead();

    Segment* segment;
    size_t endLine;
    // read from each variable
    for (size_t varIndex = 0; varIndex < variablesToRead.size(); varIndex++) {

        string symbolicName = variablesToRead[varIndex];
        string ncVariableName = dict.getNcVarName(symbolicName);
        string fileName = dict.getNcFileNameForSymbolicName(symbolicName);

        int ncId = findFile(sourceDir, fileName);

        // getting the netCDF-variable-id
        int varId;
        nc_inq_varid(ncId, ncVariableName.c_str(), &varId);

        // getting the number of dimensions of the variable
        int dimCount;
        nc_inq_varndims(ncId, varId, &dimCount);

        // getting the dimension-ids
        // TODO - replace by valarray or vector
        int* dimensionIds = new int[dimCount];
        nc_inq_vardimid(ncId, varId, dimensionIds);

        // getting the dimension sizes
        size_t camCount = 1;
        size_t lineCount = 1;
        size_t colCount = 1;

        if (dimCount == 3) {
            nc_inq_dimlen(ncId, dimensionIds[0], &camCount);
            nc_inq_dimlen(ncId, dimensionIds[1], &lineCount);
            nc_inq_dimlen(ncId, dimensionIds[2], &colCount);
        } else if (dimCount == 2) {
            nc_inq_dimlen(ncId, dimensionIds[0], &lineCount);
            nc_inq_dimlen(ncId, dimensionIds[1], &colCount);
        } else if (dimCount == 1) {
            nc_inq_dimlen(ncId, dimensionIds[0], &lineCount);
        }

        if (!context.hasSegment("SYN_COLLOCATED")) {
            // initial reading
            size_t sizeL = min(stepSize, lineCount);

            Variable& variable = dict.getVariable(symbolicName);
            nc_type type = addTypeToVariable(ncId, varId, variable);
            // this line only for debbuging reasons
            addDimsToVariable(variable, camCount, lineCount, colCount);

            segment = &context.addSegment("SYN_COLLOCATED", sizeL, colCount, camCount, 0, lineCount - 1);
            olciGrid = &segment->getGrid();
            Logger::get()->progress("Reading data for variable " + symbolicName + " into segment [" + segment->toString() + "]", getId());

            IOUtils::addVariableToSegment(symbolicName, type, *segment);
            const size_t* countVector = IOUtils::createCountVector(dimCount, camCount, sizeL, colCount);
            IOUtils::readData(ncId, varId, symbolicName, *segment, dimCount, 0, countVector);

            endLine = sizeL - 1;
        } else {
            segment = &context.getSegment("SYN_COLLOCATED");
            if (context.hasMaxLComputed(*segment, *this)) {
                modifySegmentBounds(context, *segment);
            }

            if (!segment->hasVariable(symbolicName)) {
                Variable& variable = dict.getVariable(symbolicName);
                nc_type type = addTypeToVariable(ncId, varId, variable);
                // this line only for debbuging reasons
                addDimsToVariable(variable, camCount, lineCount, colCount);

                IOUtils::addVariableToSegment(symbolicName, type, *segment);
            }

            // set up own grid for each variable, test if it is equal to OLCI-grid
            // then use OLCI or own

            // modifying segment values
            const Grid& grid = segment->getGrid();
            const size_t startLine = getStartL(context, *segment);
            endLine = getDefaultEndL(startLine, grid);
            size_t lines = endLine - startLine + 1;

            const size_t* countVector = IOUtils::createCountVector(dimCount, camCount, lines, colCount);

            Logger::get()->progress("Reading data for variable " + symbolicName + " into segment [" + segment->toString() + "]", getId());
            IOUtils::readData(ncId, varId, symbolicName, *segment, dimCount,
                    startLine, countVector);
            if( !areGridsEqual(grid, *olciGrid) ) {
//                collocate
            }
        }
    }
    context.setMaxLComputed(*segment, *this, endLine);
}

void Reader::modifySegmentBounds(const Context& context, Segment& segment) {
    size_t minRequiredLine = context.getMinLRequired(segment, context.getMaxLComputed(segment, *this) + 1);
    segment.setStartL(minRequiredLine);
}

const int Reader::findFile(string& sourceDir, string& fileName) {
    vector<string> fileNames = IOUtils::getFiles(sourceDir);

    if( openedFiles.find(fileName) != openedFiles.end() ) {
        return openedFiles[fileName];
    }

    for (size_t i = 0; i < fileNames.size(); i++) {
        string currentFileName = sourceDir + "/" + fileNames[i];
        if (boost::ends_with(currentFileName, ".nc")) {
            if (boost::ends_with(currentFileName, fileName + ".nc") ||
                    boost::ends_with(currentFileName, fileName)) {
                int ncId;
                if (nc_open(currentFileName.c_str(), NC_NOWRITE, &ncId) != NC_NOERR) {
                    throw std::runtime_error("Unable to open netCDF-file " + currentFileName + ".");
                } else {
                    openedFiles[fileName] = ncId;
                    return ncId;
                }
            }
        }
    }
    throw std::runtime_error("No file with name " + fileName + " found.");
}

const nc_type Reader::addTypeToVariable(int ncId, int varId, Variable& variable) {
    nc_type type;
    nc_inq_vartype(ncId, varId, &type);
    variable.setType(type);
    return type;
}

const bool Reader::areGridsEqual(const Grid& a, const Grid& b) const {
    return a.getSize() == b.getSize() &&
            a.getSizeK() == b.getSizeK() &&
            a.getSizeL() == b.getSizeL() &&
            a.getSizeM() == b.getSizeM() &&
            a.getStartK() == b.getStartK() &&
            a.getStartL() == b.getStartL() &&
            a.getStartM() == b.getStartM();
}

// needed only for debugging purposes
void Reader::addDimsToVariable(Variable& variable, size_t camCount, size_t lineCount, size_t colCount) {
    variable.addDimension(new Dimension("N_CAM", camCount));
    variable.addDimension(new Dimension("N_LINE_OLC", lineCount));
    variable.addDimension(new Dimension("N_DET_CAM", colCount));
}