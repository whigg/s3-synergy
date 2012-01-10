/*
 * Vac.cpp
 *
 *  Created on: Jan 09, 2012
 *      Author: thomasstorm
 */

#include <cmath>

#include "Aco.h"
#include "Vac.h"

using std::abs;
using std::exp;
using std::fill;
using std::invalid_argument;
using std::min;

Vac::Vac() : BasicModule("VPR"),
        vgtReflectanceAccessors(4),
        cO3(4),
        coordinates(8),
        f(8) {
}

Vac::~Vac() {
}

void Vac::start(Context& context) {
    collocatedSegment = &context.getSegment(Constants::SEGMENT_SYN_COLLOCATED);
    setupAccessors();
}

void Vac::setupAccessors() {
    vgtReflectanceAccessors[0] = &collocatedSegment->getAccessor("B0");
    vgtReflectanceAccessors[1] = &collocatedSegment->getAccessor("B2");
    vgtReflectanceAccessors[2] = &collocatedSegment->getAccessor("B3");
    vgtReflectanceAccessors[3] = &collocatedSegment->getAccessor("MIR");
}

void Vac::prepareAuxdata(Context& context) {
    const AuxdataProvider& vgtRadiativeTransferAuxdata = getAuxdataProvider(context, Constants::AUX_ID_VSRTAX);
    vgtRadiativeTransferAuxdata.getVectorDouble("C_O3", cO3);

    const string vgtLookupTableFile = "S3__SY_2_" + Constants::AUX_ID_VSRTAX + ".nc";
    lutRhoAtm = &getLookupTable(context, vgtLookupTableFile, "rho_atm");
    lutRatm = &getLookupTable(context, vgtLookupTableFile, "VGT_R_atm");
    lutT = &getLookupTable(context, vgtLookupTableFile, "t");
}

void Vac::process(Context& context) {
    Pixel pixel;
    valarray<double> w(lutRatm->getScalarWorkspaceSize());
    const Grid& collocatedGrid = collocatedSegment->getGrid();
    const long firstK = collocatedGrid.getFirstK();
    const long lastK = collocatedGrid.getMaxK();
    const long firstL = context.getFirstComputableL(*collocatedSegment, *this);
    const long lastL = context.getLastComputableL(*collocatedSegment, *this);
    const long firstM = collocatedGrid.getFirstM();
    const long lastM = collocatedGrid.getMaxM();
    for (long k = firstK; k <= lastK; k++) {
        for (long l = firstL; l <= lastL; l++) {
            for (long m = firstM; m <= lastM; m++) {
                const long index = collocatedGrid.getIndex(k, l, m);
                setupPixel(pixel, index);
                computeSDR(pixel, w);
                for(size_t b = 0; b < vgtReflectanceAccessors.size(); b++) {
                    vgtReflectanceAccessors[b]->setDouble(index, pixel.radiances[b]);
                }
            }
        }
    }
}

void Vac::setupPixel(Pixel& pixel, long index) {
    // todo - implement
}

void Vac::computeSDR(Pixel& p, valarray<double>& w) {
    const double D2R = 3.14159265358979323846 / 180.0;
    // todo - verify vzaOlc
    const double airMass = 0.5 * (1.0 / cos(p.sza * D2R) + 1.0 / cos(p.vzaOlc * D2R));
    for(size_t b = 0; b < vgtReflectanceAccessors.size(); b++) {
        if(p.radiances[b] == Constants::FILL_VALUE_DOUBLE) {
            continue;
            coordinates[0] = p.airPressure;
            coordinates[1] = p.waterVapour;
            coordinates[2] = p.aot;
            coordinates[3] = p.aerosolModel;
            coordinates[4] = b;
            const double rhoAtm = lutRhoAtm->getScalar(&coordinates[0], f, w);
            // todo - verify vzaOlc
            coordinates[0] = p.vzaOlc - p.sza;
            coordinates[1] = p.sza;
            // todo - verify vzaOlc
            coordinates[2] = p.vzaOlc;
            coordinates[3] = p.airPressure;
            coordinates[4] = p.waterVapour;
            coordinates[5] = p.aot;
            coordinates[6] = p.aerosolModel;
            coordinates[7] = b;
            const double rAtm = lutRatm->getScalar(&coordinates[0], f, w);
            coordinates[0] = p.sza;
            coordinates[1] = p.airPressure;
            coordinates[2] = p.waterVapour;
            coordinates[3] = p.aot;
            coordinates[4] = p.aerosolModel;
            coordinates[5] = b;
            const double tSun = lutT->getScalar(&coordinates[0], f, w);
            // todo - verify vzaOlc
            coordinates[0] = p.vzaOlc;
            const double tView = lutT->getScalar(&coordinates[0], f, w);
            const double tO3 = exp(-airMass * p.ozone * cO3[b]);
            p.radiances[b] = Aco::surfaceReflectance(p.radiances[b], rAtm, tSun, tView, rhoAtm, tO3);
        }
    }
}
