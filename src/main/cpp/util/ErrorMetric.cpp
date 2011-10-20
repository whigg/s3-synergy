/*
 * ErrorMetric.cpp
 *
 *  Created on: 13.10.2011
 *      Author: thomasstorm
 */

#include "ErrorMetric.h"
#include "../core/LookupTable.h"
#include "AuxdataProvider.h"

ErrorMetric::ErrorMetric(AerPixel& p, int16_t amin, Context& context) :
        p(p), amin(amin), context(context), D(6), lutTotalAngularWeights((ScalarLookupTable<double>&)context.getObject("weight_ang_tot")) {

    const VectorLookupTable<double>& lutD = (VectorLookupTable<double>&)context.getObject("D");
    valarray<double> coordinates(lutD.getDimensionCount());
    assert(coordinates.size() == 4);
    coordinates[0] = p.sza;
    coordinates[1] = p.airPressure;
    coordinates[2] = p.getTau550();
    coordinates[3] = amin;

    lutD.getValues(&coordinates[0], D);

    AuxdataProvider& configurationAuxdata = (AuxdataProvider&)context.getObject(Constants::AUXDATA_CONFIGURATION_ID);
    spectralWeights = configurationAuxdata.getDoubleArray("weight_spec");
    vegetationSpectrum = configurationAuxdata.getDoubleArray("R_veg");
    soilReflectance = configurationAuxdata.getDoubleArray("R_soil");
    ndviIndices = configurationAuxdata.getShortArray("NDV_channel");
    angularWeights = configurationAuxdata.getDoubleMatrix("weight_ang");
    gamma = configurationAuxdata.getDouble("gamma");
}

double ErrorMetric::value(valarray<double>& x) {
    for (size_t i = 1; i <= 30; i++) {
        if (p.isFillValue("L_" + lexical_cast<string>(i))) {
            spectralWeights[i - 1] = 0;
            if (i >= 19 && i <= 24) {
                angularWeights.insert_element(i - 19, 0, 0.0);
            } else if (i >= 25 && i <= 30) {
                angularWeights.insert_element(i - 25, 1, 0.0);
            }
        }
    }
    applyAtmosphericCorrection(p, amin);

    valarray<double> rSpec(30);
    valarray<double> rAng(12);
    for (size_t i = 0; i < 30; i++) {
        rSpec[i] = specModelSurf(x[0], x[1], i);
    }
    for (size_t i = 0; i < 12; i++) {
        rAng[i] = angModelSurf(i, x);
    }
    return errorMetric(rSpec, rAng);
}

static double ozoneTransmission(double cO3, double sza, double vza, double nO3) {
    static const double D2R = 3.14159265358979323846 / 180.0;
    // Eq. 2-2
    const double m = 0.5 * (1.0 / std::cos(sza * D2R) + 1.0 / std::cos(vza * D2R));
    const double tO3 = std::exp(-m * nO3 * cO3);

    return tO3;
}

static double toaReflectance(double ltoa, double f0, double sza) {
    static const double D2R = 3.14159265358979323846 / 180.0;
    static const double PI = 3.14159265358979323846;
    return (PI * ltoa) / (f0 * std::cos(sza * D2R));
}

static double surfaceReflectance(double rtoa, double ratm, double ts, double tv, double rho, double tO3) {
    // Eq. 2-3
    const double f = (rtoa - tO3 * ratm) / (tO3 * ts * tv);
    const double rboa = f / (1.0 + rho * f);

    return rboa;
}

void ErrorMetric::applyAtmosphericCorrection(AerPixel& p, int16_t amin) {

    const MatrixLookupTable<double>& lutOlcRatm = (MatrixLookupTable<double>&) context.getObject("OLC_R_atm");
    const MatrixLookupTable<double>& lutSlnRatm = (MatrixLookupTable<double>&) context.getObject("SLN_R_atm");
    const MatrixLookupTable<double>& lutSloRatm = (MatrixLookupTable<double>&) context.getObject("SLO_R_atm");
    const MatrixLookupTable<double>& lutT = (MatrixLookupTable<double>&) context.getObject("t");
    const MatrixLookupTable<double>& lutRhoAtm = (MatrixLookupTable<double>&) context.getObject("rho_atm");

    valarray<double> coordinates(10);

    coordinates[0] = std::abs(p.saa - p.vaaOlc); // ADA
    coordinates[1] = p.sza; // SZA
    coordinates[2] = p.vzaOlc; // VZA
    coordinates[3] = p.airPressure; // air pressure
    coordinates[4] = p.waterVapour; // water vapour
    coordinates[5] = p.getTau550(); // aerosol

    coordinates[6] = coordinates[1]; // SZA
    coordinates[7] = coordinates[3]; // air pressure
    coordinates[8] = coordinates[4]; // water vapour
    coordinates[9] = coordinates[5]; // aerosol

    matrix<double> matRatmOlc(40, 18);
    matrix<double> matRatmSln(40, 6);
    matrix<double> matRatmSlo(40, 6);
    matrix<double> matTs(40, 30);
    matrix<double> matTv(40, 30);
    matrix<double> matRho(40, 30);

    valarray<double> f(lutOlcRatm.getDimensionCount());
    valarray<double> w(lutOlcRatm.getWorkspaceSize());

    lutOlcRatm.getValues(&coordinates[0], matRatmOlc, f, w);
    lutT.getValues(&coordinates[6], matTs, f, w);
    lutT.getValues(&coordinates[2], matTv, f, w);
    lutRhoAtm.getValues(&coordinates[3], matRho, f, w);

    for(size_t b = 0; b < 19; b++) {
        // Eq. 2-1
        const double rtoa = toaReflectance(p.getRadiance(b), p.solarIrradiances[b], p.sza);

        // Eq. 2-2
        const double tO3 = ozoneTransmission(p.cO3[b + 1.0], p.sza, p.vzaOlc, p.ozone);

        // Eq. 2-3
        const double ratm = matRatmOlc(amin - 1, b);
        const double ts = matTs(amin - 1, b);
        const double tv = matTv(amin - 1, b);
        const double rho = matRho(amin - 1, b);
        const double sdr = surfaceReflectance(rtoa, ratm, ts, tv, rho, tO3);

        if (sdr >= 0.0 && sdr <= 1.0) {
            p.setSDR(b + 1, sdr);
        } else {
            // todo - replace lexical cast
            p.setFillValue("SDR_" + lexical_cast<string>(b + 1));
        }
    }
}

double ErrorMetric::angModelSurf(size_t index, valarray<double>& x) {
    const size_t j = index % 6;
    const size_t o = index / 6;
    const double d = D[j];
    const double omega = x[j + 4];
    const double nu = x[o + 2];
    const double g = (1 - gamma) * omega;
    return (1 - d) * nu * omega + (gamma * omega) / (1 - g) * (d + g * (1 - d));
}

double ErrorMetric::specModelSurf(double c_1, double c_2, size_t index) {
    return c_1 * vegetationSpectrum[index] + c_2 * soilReflectance[index];
}

double ErrorMetric::errorMetric(valarray<double> rSpec, valarray<double> rAng) {
    double ndvi = ndv(p, ndviIndices);
    double sum1 = 0.0;
    double sum2 = 0.0;
    double sum3 = 0.0;
    double sum4 = 0.0;
    for (size_t i = 0; i < 30; i++) {
        sum1 += spectralWeights[i] * (p.getSDR(i) - rSpec[i]) * (p.getSDR(i) - rSpec[i]);
    }
    for (size_t i = 0; i < 30; i++) {
        sum2 += spectralWeights[i];
    }
    for (size_t i = 0; i < 12; i++) {
        size_t xIndex = i < 6 ? 0 : 1;
        size_t yIndex = i % 6;
        sum3 += angularWeights.at_element(xIndex, yIndex) * (p.getSDR(i) - rAng[i]) * (p.getSDR(i) - rAng[i]);
    }
    for (size_t i = 0; i < 12; i++) {
        size_t xIndex = i < 6 ? 0 : 1;
        size_t yIndex = i % 6;
        sum4 += angularWeights.at_element(xIndex, yIndex);
    }
    const double totalAngularWeight = lutTotalAngularWeights.getValue(&ndvi);
    return (1 - totalAngularWeight) * sum1 / sum2 + totalAngularWeight * sum3 / sum4;
}

double ErrorMetric::ndv(AerPixel& q, valarray<int16_t> ndviIndices) {
    double L1 = q.getRadiance(ndviIndices[0]);
    double L2 = q.getRadiance(ndviIndices[1]);
    double F1 = q.solarIrradiances[ndviIndices[0]];
    double F2 = q.solarIrradiances[ndviIndices[1]];
    if (q.isFillValue("L_" + lexical_cast<string>(ndviIndices[0])) ||
            q.isFillValue("L_" + lexical_cast<string>(ndviIndices[1])) ||
            isSolarIrradianceFillValue(F1, q.solarIrradianceFillValues, ndviIndices[0]) ||
            isSolarIrradianceFillValue(F2, q.solarIrradianceFillValues, ndviIndices[1])) {
        return 0.5;
    }
    return ((L2 / F2) - (L1 / F1)) / ((L2 / F2) + (L1 / F1));
}

bool ErrorMetric::isSolarIrradianceFillValue(double f, const valarray<double> fillValues, int16_t index) {
    if (index >= 18) {
        return false;
    }
    return f == fillValues[index];
}
