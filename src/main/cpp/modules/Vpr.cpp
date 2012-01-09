/*
 * Vpr.cpp
 *
 *  Created on: Dec 01, 2011
 *      Author: thomasstorm
 */

#include "Vpr.h"

using std::abs;
using std::fill;
using std::invalid_argument;
using std::min;

Vpr::Vpr() : BasicModule("VPR"),
        collocatedReflectanceAccessors(4),
        vgtReflectanceAccessors(4) {
}

Vpr::~Vpr() {
}

void Vpr::start(Context& context) {
    collocatedSegment = &context.getSegment(Constants::SEGMENT_SYN_COLLOCATED);
    geoSegment = &context.getSegment(Constants::SEGMENT_GEO);
    geoGrid = &geoSegment->getGrid();
    latAccessor = &geoSegment->getAccessor("latitude");
    lonAccessor = &geoSegment->getAccessor("longitude");

    context.getLogging().info("Adding segment '" + Constants::SEGMENT_VGT_PLATE_CARREE + "' to context.", getId());
    vgtSegment = &context.addSegment(Constants::SEGMENT_VGT_PLATE_CARREE, 400, COL_COUNT, 1, 0, LINE_COUNT - 1);

    setupAccessors();
}

void Vpr::setupAccessors() {
    collocatedReflectanceAccessors[0] = &collocatedSegment->getAccessor("B0");
    collocatedReflectanceAccessors[1] = &collocatedSegment->getAccessor("B2");
    collocatedReflectanceAccessors[2] = &collocatedSegment->getAccessor("B3");
    collocatedReflectanceAccessors[3] = &collocatedSegment->getAccessor("MIR");
    collocatedFlagsAccessor = &collocatedSegment->getAccessor("SM");

    vgtReflectanceAccessors[0] = &vgtSegment->addVariable("B0", Constants::TYPE_DOUBLE);
    vgtReflectanceAccessors[1] = &vgtSegment->addVariable("B2", Constants::TYPE_DOUBLE);
    vgtReflectanceAccessors[2] = &vgtSegment->addVariable("B3", Constants::TYPE_DOUBLE);
    vgtReflectanceAccessors[3] = &vgtSegment->addVariable("MIR", Constants::TYPE_DOUBLE);
    vgtFlagsAccessor = &vgtSegment->addVariable("SM", Constants::TYPE_UBYTE);
}

void Vpr::process(Context& context) {
    const long firstL = context.getFirstComputableL(*vgtSegment, *this);
    const long lastL = context.getLastComputableL(*vgtSegment, *this);

    double synMinLat = 90.0;
    double synMaxLat = -90.0;
    minMaxSynLat(&synMinLat, &synMaxLat);
    double vgtMinLat = 90.0;
    double vgtMaxLat = -90.0;
    minMaxVgtLat(firstL, lastL, &vgtMinLat, &vgtMaxLat);
    /*
     * if the minimum latitude of the current VGT segment is greater than the maximum latitude
     * of the current SYN segment, move the VGT segment by setting its last computed line
     * and do not allow moving of the SYN segment by setting its first required line.
     */
    if(synMaxLat < vgtMinLat) {
        context.setLastComputedL(*vgtSegment, *this, lastL);
//        const long currentFirstRequiredLine = context.getFirstRequiredL(*collocatedSegment);
        const long currentFirstRequiredLine = context.getFirstComputableL(*collocatedSegment, *this);
        context.setFirstRequiredL(*collocatedSegment, *this, currentFirstRequiredLine);
        return;
    }
    /*
     * if the maximum latitude of the current VGT segment is less than the minimum latitude
     * of the current SYN segment, move the SYN segment by setting its first required line
     * and do not allow moving of the VGT segment by setting its last computed line.
     */
    if(vgtMaxLat < synMinLat) {
        context.setLastComputedL(*vgtSegment, *this, firstL);
        const long firstRequiredLine = collocatedSegment->getGrid().getMaxL();
        context.setFirstRequiredL(*collocatedSegment, *this, firstRequiredLine);
        return;
    }

    /*
     * if the minimum latitude of the current VGT segment is less than the maximum latitude
     * of the current SYN segment AND the minimum VGT latitude is greater than the minumum
     * SYN latitude (i.e. the VGT segment and the SYN segment overlap), perform the reprojection
     * and set the first required line of the SYN segment to the line of the SYN segment nearest
     * to vgtMaxLat.
     */

    long l = findLineOfSynSegmentNearestTo(vgtMaxLat);

    context.setFirstRequiredL(*collocatedSegment, *this, l);

    double synMinLon = 180.0;
    double synMaxLon = -180.0;
    minMaxSynLon(&synMinLon, &synMaxLon);

    const Grid& vgtGrid = vgtSegment->getGrid();
    const Grid& synGrid = collocatedSegment->getGrid();
    valarray<long> synIndices(3);
    bool indicesFound = false;
    for(long l = firstL; l <= lastL; l++) {
        const double lat = getVgtLatitude(l);
        if(lat > synMaxLat || lat < synMinLat) {
            continue;
        }
        context.getLogging().info("Projecting vgt line " + lexical_cast<string>(l), getId());
        for(long m = vgtGrid.getFirstM(); m <= vgtGrid.getMaxM(); m++) {
            const double lon = getVgtLongitude(m);
            if(synMinLon > lon || synMaxLon < lon) {
                continue;
            }
            if(indicesFound) {
                findPixelPosAroundGivenIndices(lat, lon, synIndices);
            } else {
                findPixelPosInWholeGrid(lat, lon, synIndices);
            }
            const long synK = synIndices[0];
            const long synL = synIndices[1];
            const long synM = synIndices[2];
            if (synGrid.isValidPosition(synK, synL, synM)) {
                setValues(synK, synL, synM, l, m);
                indicesFound = true;
            } else {
                indicesFound = false;
                continue;
            }
        }
    }
}

void Vpr::minMaxSynLat(double* minLat, double* maxLat) const {
    for(long k = geoGrid->getFirstK(); k <= geoGrid->getMaxK(); k++) {
        for(long l = geoGrid->getFirstL(); l <= geoGrid->getMaxL(); l++) {
            for(long m = geoGrid->getFirstM(); m <= geoGrid->getMaxM(); m++) {
                const size_t index = geoGrid->getIndex(k, l , m);
                const double lat = latAccessor->getDouble(index);
                if(lat < *minLat) {
                    *minLat = lat;
                }
                if(lat > *maxLat) {
                    *maxLat = lat;
                }
            }
        }
    }
}

void Vpr::minMaxSynLon(double* minLon, double* maxLon) const {
    for(long k = geoGrid->getFirstK(); k <= geoGrid->getMaxK(); k++) {
        for(long l = geoGrid->getFirstL(); l <= geoGrid->getMaxL(); l++) {
            for(long m = geoGrid->getFirstM(); m <= geoGrid->getMaxM(); m++) {
                const size_t index = geoGrid->getIndex(k, l , m);
                const double lon = lonAccessor->getDouble(index);
                if(lon < *minLon) {
                    *minLon = lon;
                }
                if(lon > *maxLon) {
                    *maxLon = lon;
                }
            }
        }
    }
}

void Vpr::minMaxVgtLat(long firstL, long lastL, double* minLat, double* maxLat) const {
    const Grid& vgtGrid = vgtSegment->getGrid();
    for(long l = firstL; l <= lastL; l++) {
        const double lat = getVgtLatitude(l);
        if(lat < *minLat) {
            *minLat = lat;
        }
        if(lat > *maxLat) {
            *maxLat = lat;
        }
    }
}

void Vpr::findPixelPosInWholeGrid(double lat, double lon, valarray<long>& synIndices) const {
    findPixelPos(lat, lon,
            geoGrid->getFirstK(), geoGrid->getMaxK(),
            geoGrid->getFirstL(), geoGrid->getMaxL(),
            geoGrid->getFirstM(), geoGrid->getMaxM(),
            synIndices);
}

void Vpr::findPixelPosAroundGivenIndices(double lat, double lon, valarray<long>& synIndices) const {
    const long k0 = max<long>(0, synIndices[0] - PIXEL_SEARCH_RADIUS / 2);
    const long kMax = min<long>(geoGrid->getMaxK(), synIndices[0] + PIXEL_SEARCH_RADIUS / 2);
    const long l0 = max<long>(0, synIndices[1] - PIXEL_SEARCH_RADIUS / 2);
    const long lMax = min<long>(geoGrid->getMaxL(), synIndices[1] + PIXEL_SEARCH_RADIUS / 2);
    const long m0 = max<long>(0, synIndices[2] - PIXEL_SEARCH_RADIUS / 2);
    const long mMax = min<long>(geoGrid->getMaxM(), synIndices[2] + PIXEL_SEARCH_RADIUS / 2);
    findPixelPos(lat, lon, k0, kMax, l0, lMax, m0, mMax, synIndices);
}

void Vpr::findPixelPos(double lat, double lon, long k0, long kMax, long l0, long lMax, long m0, long mMax, valarray<long>& synIndices) const {
    double synLat;
    double synLon;
    double delta = numeric_limits<double>::max();
    for (long k = k0; k <= kMax; k++) {
        for (long l = l0; l <= lMax; l++) {
            for (long m = m0; m <= mMax; m++) {
                const size_t index = geoGrid->getIndex(k, l, m);
                synLat = latAccessor->getDouble(index);
                synLon = lonAccessor->getDouble(index);
                double innerDelta = std::abs(lat - synLat) + std::abs(lon - synLon);
                if(innerDelta < delta) {
                    delta = innerDelta;
                    synIndices[0] = k;
                    synIndices[1] = l;
                    synIndices[2] = m;
                }
            }
        }
    }
}

void Vpr::setValues(long synK, long synL, long synM, long l, long m) {
    const Grid& collocatedGrid = collocatedSegment->getGrid();
    const Grid& vgtGrid = vgtSegment->getGrid();
    const size_t collocatedIndex = collocatedGrid.getIndex(synK, synL, synM);
    const size_t vgtIndex = vgtGrid.getIndex(0, l, m);
    for(size_t i = 0; i < collocatedReflectanceAccessors.size(); i++) {
        const double value = collocatedReflectanceAccessors[i]->getDouble(collocatedIndex);
        vgtReflectanceAccessors[i]->setDouble(vgtIndex, value);
    }
    const uint8_t flags = collocatedFlagsAccessor->getUByte(collocatedIndex);
    vgtFlagsAccessor->setUByte(vgtIndex, flags);
}

double Vpr::getVgtLatitude(long l) {
    if(l < 0 || l >= LINE_COUNT) {
        BOOST_THROW_EXCEPTION(std::invalid_argument("l < 0 || l >= LINE_COUNT, l = " + lexical_cast<string>(l)));
    }
    const double unnormalisedResult = l * PIXEL_SIZE;
    return 75 - unnormalisedResult;
}

double Vpr::getVgtLongitude(long m) {
    if(m < 0 || m >= COL_COUNT) {
        BOOST_THROW_EXCEPTION(std::invalid_argument("m < 0 || m >= COL_COUNT, m = " + lexical_cast<string>(m)));
    }
    const double unnormalisedResult = m * PIXEL_SIZE;
    return -180 + unnormalisedResult;
}

long Vpr::findLineOfSynSegmentNearestTo(double vgtMaxLat) const {
    long result;
    double delta = numeric_limits<double>::max();
    for (long k = geoGrid->getFirstK(); k <= geoGrid->getMaxK(); k++) {
        for (long l = geoGrid->getFirstL(); l <= geoGrid->getMaxL(); l++) {
            for (long m = geoGrid->getFirstM(); m <= geoGrid->getMaxM(); m++) {
                const size_t index = geoGrid->getIndex(k, l, m);
                double synLat = latAccessor->getDouble(index);
                double innerDelta = std::abs(vgtMaxLat - synLat);
                if(innerDelta < delta) {
                    delta = innerDelta;
                    result = l;
                }
            }
        }
    }
    return result;
}
