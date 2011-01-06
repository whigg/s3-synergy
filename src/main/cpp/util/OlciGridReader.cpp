/*
 * File:   Reader.cpp
 * Author: thomass
 *
 * Created on November 18, 2010, 2:08 PM
 */

#include <algorithm>
#include <boost/algorithm/string/predicate.hpp>
#include <iostream>
#include <valarray>
#include <stdexcept>
#include <netcdf.h>
#include <stddef.h>

#include "JobOrder.h"
#include "Logger.h"
#include "OlciGridReader.h"
#include "OlciGridReaderConstants.h"
#include "IOUtils.h"

using std::min;
using std::max;

OlciGridReader::OlciGridReader() : DefaultModule("READ"), stepSize(5000) {
}

OlciGridReader::~OlciGridReader() {
}

void OlciGridReader::process(Context& context) {
    // switch between processor configurations

    Dictionary& dict = *(context.getDictionary());
    string processorId = "SYL2";
    string sourceDir = context.getJobOrder()->getProcessorConfiguration(processorId).getInputList()[0]->getFileNames()[0];
    const vector<string>& variablesToRead = OlciGridReaderConstants::getVariablesToRead();

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
        int* dimensionIds = new int[dimCount];
        nc_inq_vardimid(ncId, varId, dimensionIds);

        // getting the dimension sizes
        size_t camCount = 1;
        size_t lineCount = 1;
        size_t colCount = 1;
        nc_inq_dimlen(ncId, dimensionIds[0], &camCount);
        nc_inq_dimlen(ncId, dimensionIds[1], &lineCount);
        nc_inq_dimlen(ncId, dimensionIds[2], &colCount);

        if (!context.hasSegment("SYN_COLLOCATED")) {
            size_t sizeL = min(stepSize, lineCount);
            // initial reading

            Variable& variable = dict.getVariable(symbolicName);
            nc_type type = addTypeToVariable(ncId, varId, variable);
            addDimsToVariable(variable, camCount, lineCount, colCount);

            segment = &context.addSegment("SYN_COLLOCATED", sizeL, colCount, camCount, 0, lineCount - 1);
            Logger::get()->progress("Reading data for variable " + symbolicName + " into segment [" + segment->toString() + "]", getId());

            IOUtils::addVariableToSegment(symbolicName, type, *segment);
            const size_t* countVector = IOUtils::createCountVector(dimCount, camCount, sizeL, colCount);
            IOUtils::readData(ncId, varId, symbolicName, *segment, dimCount, 0, countVector, 0);

            endLine = sizeL - 1;
        } else {
            segment = &context.getSegment("SYN_COLLOCATED");
            if (context.hasMaxLComputed(*segment, *this)) {
                modifySegmentBounds(context, *segment);
            }

            if (!segment->hasVariable(symbolicName)) {
                Variable& variable = dict.getVariable(symbolicName);
                nc_type type = addTypeToVariable(ncId, varId, variable);
                addDimsToVariable(variable, camCount, lineCount, colCount);
                IOUtils::addVariableToSegment(symbolicName, type, *segment);
            }

            // modifying segment values
            Grid& grid = segment->getGrid();
            Logger::get()->progress("Reading data for variable " + symbolicName + " into segment [" + segment->toString() + "]", getId());
            size_t startLine = getStartL(context, *segment);
            endLine = getDefaultEndL(startLine, grid);
            size_t lines = endLine - startLine + 1;

            const size_t* countVector = IOUtils::createCountVector(dimCount, camCount, lines, colCount);
            IOUtils::readData(ncId, varId, symbolicName, *segment,
                    dimCount, startLine, countVector, grid.getIndex(0, startLine, 0));
        }
    }
    context.setMaxLComputed(*segment, *this, endLine);
}

void OlciGridReader::modifySegmentBounds(const Context& context, Segment& segment) {
    size_t minRequiredLine = context.getMinLRequired(segment, context.getMaxLComputed(segment, *this) + 1);
    segment.setStartL(minRequiredLine);
}

const int OlciGridReader::findFile(string& sourceDir, string& fileName) {
    vector<string> fileNames = IOUtils::getFiles(sourceDir);

    // get Variables from every file
    for (size_t i = 0; i < fileNames.size(); i++) {
        string currentFileName = sourceDir + "/" + fileNames[i];
        if (boost::ends_with(currentFileName, fileName) + ".nc" ||
                boost::ends_with(currentFileName, fileName)) {
            int ncId;
            if (nc_open(currentFileName.c_str(), NC_NOWRITE, &ncId) != NC_NOERR) {
                throw std::runtime_error("Unable to open netCDF-file " + currentFileName + ".");
            } else {
                return ncId;
            }
        }
    }
    throw std::runtime_error("No file with name " + fileName + " found.");
}

const nc_type OlciGridReader::addTypeToVariable(int ncId, int varId, Variable& variable) {
    nc_type type;
    nc_inq_vartype(ncId, varId, &type);
    variable.setType(type);
    return type;
}

void OlciGridReader::addDimsToVariable(Variable& variable, size_t camCount, size_t lineCount, size_t colCount) {
    variable.addDimension(new Dimension("N_CAM", camCount));
    variable.addDimension(new Dimension("N_LINE_OLC", lineCount));
    variable.addDimension(new Dimension("N_DET_CAM", colCount));
}
