/*
 * Vbm.cpp
 *
 *  Created on: Nov 17, 2011
 *      Author: thomasstorm
 */

#include "Aer.h"
#include "Vbm.h"

Vbm::Vbm() :
		BasicModule("PCL"), vgtBSrfLuts(4), synRadianceAccessors(30), synSolarIrradianceAccessors(30) {
}

Vbm::~Vbm() {
}

void Vbm::start(Context& context) {
    AuxdataProvider& radiativeTransfer = getAuxdataProvider(context, Constants::AUX_ID_VPRTAX);
    amin = radiativeTransfer.getShort("AMIN");
    collocatedSegment = &context.getSegment(Constants::SEGMENT_SYN_COLLOCATED);
    olciInfoSegment = &context.getSegment(Constants::SEGMENT_OLC_INFO);

    synLatitudeAccessor = &collocatedSegment->getAccessor("latitude");
    synLongitudeAccessor = &collocatedSegment->getAccessor("longitude");
    for(size_t i = 0; i < 30; i++) {
        const string index = lexical_cast<string>(i + 1);
        synRadianceAccessors[i] = &collocatedSegment->getAccessor("L_" + index);
        synSolarIrradianceAccessors[i] = &collocatedSegment->getAccessor("solar_irradiance_" + index);
    }

    prepareAuxdata(context);
    prepareTiePointData(context);

    addVariables();
}

void Vbm::stop(Context& context) {
}

void Vbm::prepareAuxdata(Context& context) {
    synLutRhoAtm = &getLookupTable(context, Constants::AUX_ID_SYRTAX, "rho_atm");
    synLutOlcRatm = &getLookupTable(context, Constants::AUX_ID_SYRTAX, "OLC_R_atm");
    synLutSlnRatm = &getLookupTable(context, Constants::AUX_ID_SYRTAX, "SLN_R_atm");
    synLutT = &getLookupTable(context, Constants::AUX_ID_SYRTAX, "t");
    synCo3 = &getAuxdataProvider(context, Constants::AUX_ID_SYRTAX).getVectorDouble("C_O3");

    vgtLutRhoAtm = &getLookupTable(context, Constants::AUX_ID_VPRTAX, "rho_atm");
    vgtLutRAtm = &getLookupTable(context, Constants::AUX_ID_VPRTAX, "R_atm");
    vgtLutT = &getLookupTable(context, Constants::AUX_ID_VPRTAX, "t");
    vgtCo3 = &getAuxdataProvider(context, Constants::AUX_ID_VPRTAX).getVectorDouble("C_O3");

    vgtBSrfLuts[0] = &getLookupTable(context, Constants::AUX_ID_VPSRAX, "B0_SRF");
    vgtBSrfLuts[1] = &getLookupTable(context, Constants::AUX_ID_VPSRAX, "B2_SRF");
    vgtBSrfLuts[2] = &getLookupTable(context, Constants::AUX_ID_VPSRAX, "B3_SRF");
    vgtBSrfLuts[3] = &getLookupTable(context, Constants::AUX_ID_VPSRAX, "MIR_SRF");

    vgtLutSolarIrradiance = &getLookupTable(context, Constants::AUX_ID_VPSRAX, "solar_irradiance");
}

void Vbm::prepareTiePointData(Context& context) {
    const Segment& olciTiepointSegment = context.getSegment(Constants::SEGMENT_OLC_TP);
    const Segment& slnTiepointSegment = context.getSegment(Constants::SEGMENT_SLN_TP);

    copy(olciTiepointSegment.getAccessor("SZA").getDoubles(), (*szaOlcTiePoints));
    copy(olciTiepointSegment.getAccessor("SAA").getDoubles(), (*saaOlcTiePoints));
    copy(olciTiepointSegment.getAccessor("OLC_VZA").getDoubles(), (*vzaOlcTiePoints));
    copy(slnTiepointSegment.getAccessor("SLN_VZA").getDoubles(), (*vzaSlnTiePoints));

    copy(olciTiepointSegment.getAccessor("ozone").getDoubles(), (*ozoneTiePoints));
    copy(olciTiepointSegment.getAccessor("air_pressure").getDoubles(), (*airPressureTiePoints));

    if (olciTiepointSegment.hasVariable("water_vapour")) {
        copy(olciTiepointSegment.getAccessor("water_vapour").getDoubles(), (*waterVapourTiePoints));
    }

    const valarray<double> olciLats = olciTiepointSegment.getAccessor("OLC_TP_lat").getDoubles();
    const valarray<double> olciLons = olciTiepointSegment.getAccessor("OLC_TP_lon").getDoubles();
    const valarray<double> slnLats = slnTiepointSegment.getAccessor("SLN_TP_lat").getDoubles();
    const valarray<double> slnLons = slnTiepointSegment.getAccessor("SLN_TP_lon").getDoubles();
    tiePointInterpolatorOlc = shared_ptr<TiePointInterpolator<double > >(new TiePointInterpolator<double>(olciLons, olciLats));
    tiePointInterpolatorSln = shared_ptr<TiePointInterpolator<double > >(new TiePointInterpolator<double>(slnLons, slnLats));
}

void Vbm::addVariables() {
    vgtFlagsAccessor = &collocatedSegment->addVariable("SM", Constants::TYPE_UBYTE);
    vgtB0Accessor = &collocatedSegment->addVariable("B0", Constants::TYPE_UBYTE);
    vgtB2Accessor = &collocatedSegment->addVariable("B2", Constants::TYPE_UBYTE);
    vgtB3Accessor = &collocatedSegment->addVariable("B3", Constants::TYPE_UBYTE);
    vgtMirAccessor = &collocatedSegment->addVariable("MIR", Constants::TYPE_UBYTE);
}

void Vbm::process(Context& context) {
    const Grid& collocatedGrid = collocatedSegment->getGrid();

    // todo - verify
    long firstL = context.getFirstComputableL(*collocatedSegment, *this);
    long lastL = context.getLastComputableL(*collocatedSegment, *this);

    valarray<double> surfaceReflectances(24);
    valarray<double> hyperSpectralReflectances(914);
    valarray<double> toaReflectances(914);
    valarray<double> vgtToaReflectances(4);

    Pixel p;

    for(long l = firstL; l <= lastL; l++) {
        for(long m = collocatedGrid.getFirstM(); m <= collocatedGrid.getMaxM(); m++) {
            for(long k = collocatedGrid.getFirstK(); k <= collocatedGrid.getMaxK(); k++) {
                const size_t index = collocatedGrid.getIndex(k, l, m);

                setupPixel(p, index);
                p.aot = computeT550(p.lat);
                downscale(p, surfaceReflectances);
                performHyperspectralInterpolation(k, m, context, surfaceReflectances, hyperSpectralReflectances);
                performHyperspectralUpscaling(hyperSpectralReflectances, p, toaReflectances);
                performHyperspectralFiltering(toaReflectances, vgtToaReflectances);

                const uint8_t flags = getFlagsAndFills(p, vgtToaReflectances);

                setValues(index, flags, vgtToaReflectances);

                cleanup(surfaceReflectances, hyperSpectralReflectances, toaReflectances, vgtToaReflectances);
            }
        }
    }
}

void Vbm::downscale(const Pixel& p, valarray<double>& surfReflNadirSyn) {
    valarray<double> coordinates(7);
    coordinates[0] = p.airPressure;
    coordinates[1] = p.waterVapour;
    coordinates[2] = p.aot;
    coordinates[3] = p.aerosolModel;

    valarray<double> f(synLutRhoAtm->getDimensionCount());
    valarray<double> w(synLutRhoAtm->getMatrixWorkspaceSize());

    valarray<double> rhoAtm(30);
    valarray<double> rAtmOlc(18);
    valarray<double> tSun(30);
    valarray<double> tViewOlc(30);

    synLutRhoAtm->getVector(&coordinates[0], rhoAtm, f, w);

    coordinates[0] = std::abs(p.vaaOlc - p.saa);
    coordinates[1] = p.sza;
    coordinates[2] = p.vzaOlc;
    coordinates[3] = p.airPressure;
    coordinates[4] = p.waterVapour;
    coordinates[5] = p.aot;
    coordinates[6] = p.aerosolModel;

    synLutOlcRatm->getVector(&coordinates[0], rAtmOlc, f, w);

    coordinates[0] = p.sza;
    coordinates[1] = p.airPressure;
    coordinates[2] = p.waterVapour;
    coordinates[3] = p.aot;
    coordinates[4] = p.aerosolModel;
    synLutT->getVector(&coordinates[0], tSun, f, w);

    coordinates[0] = p.vzaOlc;
    synLutT->getVector(&coordinates[0], tViewOlc, f, w);

    for(size_t i = 0; i < 18; i++) {
        surfReflNadirSyn[i] = surfaceReflectance(
                p.ozone,
                p.vzaOlc,
                p.sza,
                p.solarIrradiances[i],
                p.radiances[i],
                (*synCo3)[i],  // todo - verify!
                rhoAtm[i],
                rAtmOlc[i],
                tSun[i],
                tViewOlc[i]);
    }

    valarray<double> rAtmSln(6);

    coordinates[0] = std::abs(p.vaaSln - p.saa);
    coordinates[1] = p.sza;
    coordinates[2] = p.vzaSln;
    coordinates[3] = p.airPressure;
    coordinates[4] = p.waterVapour;
    coordinates[5] = p.aot;
    coordinates[6] = p.aerosolModel;

    synLutSlnRatm->getVector(&coordinates[0], rAtmSln, f, w);

    coordinates[0] = p.sza;
    coordinates[1] = p.airPressure;
    coordinates[2] = p.waterVapour;
    coordinates[3] = p.aot;
    coordinates[4] = p.aerosolModel;

    valarray<double> tViewSln(30);

    coordinates[0] = p.vzaSln;
    coordinates[1] = p.airPressure;
    coordinates[2] = p.waterVapour;
    coordinates[3] = p.aot;
    coordinates[4] = p.aerosolModel;
    synLutT->getVector(&coordinates[0], tViewSln, f, w);

    for(size_t i = 18; i < 24; i++) {
        surfReflNadirSyn[i] = surfaceReflectance(
                p.ozone,
                p.vzaSln,
                p.sza,
                p.solarIrradiances[i],
                p.radiances[i],
                (*synCo3)[i],  // todo - verify!
                rhoAtm[i],
                rAtmSln[i],
                tSun[i],
                tViewSln[i]);
    }
}

double Vbm::surfaceReflectance(double ozone, double vza, double sza, double solarIrradiance, double radiance,
        double co3, double rhoAtm, double rAtm, double tSun, double tView) {
    if(radiance == Constants::FILL_VALUE_DOUBLE) {
        return Constants::FILL_VALUE_DOUBLE;
    }

    double rToa = M_PI * radiance / (solarIrradiance * std::cos(sza));
    double M = 0.5 * (1/std::cos(sza) + 1/std::cos(vza));
    double t_O3 = std::exp(-M * ozone * co3);

    double f = (rToa - t_O3 * rAtm) / (t_O3 * tSun * tView);
    return f / (1 + rhoAtm * f);
}

void Vbm::setupPixel(Pixel& p, size_t index) {
    p.aerosolModel = amin;
    for(size_t i = 0; i < 30; i++) {
        p.radiances = synRadianceAccessors[i]->isFillValue(index) ? Constants::FILL_VALUE_DOUBLE : synRadianceAccessors[i]->getDouble(index);
        p.solarIrradiances = synSolarIrradianceAccessors[i]->isFillValue(index) ? Constants::FILL_VALUE_DOUBLE : synSolarIrradianceAccessors[i]->getDouble(index);
    }
    p.lat = synLatitudeAccessor->getDouble(index);
    p.lon = synLongitudeAccessor->getDouble(index);

    valarray<double> tpiWeights(1);
    valarray<size_t> tpiIndexes(1);

    tiePointInterpolatorOlc->prepare(p.lat, p.lon, tpiWeights, tpiIndexes);

    p.sza = tiePointInterpolatorOlc->interpolate((*szaOlcTiePoints), tpiWeights, tpiIndexes);
    p.saa = tiePointInterpolatorOlc->interpolate((*saaOlcTiePoints), tpiWeights, tpiIndexes);
    p.vzaOlc = tiePointInterpolatorOlc->interpolate((*vzaOlcTiePoints), tpiWeights, tpiIndexes);

    p.airPressure = tiePointInterpolatorOlc->interpolate((*airPressureTiePoints), tpiWeights, tpiIndexes);
    p.ozone = tiePointInterpolatorOlc->interpolate((*ozoneTiePoints), tpiWeights, tpiIndexes);
    if (waterVapourTiePoints->size() != 0) {
        p.waterVapour = tiePointInterpolatorOlc->interpolate((*waterVapourTiePoints), tpiWeights, tpiIndexes);
    } else {
        p.waterVapour = 0.2;
    }

    tiePointInterpolatorSln->prepare(p.lat, p.lon, tpiWeights, tpiIndexes);
    p.vzaSln = tiePointInterpolatorSln->interpolate((*vzaSlnTiePoints), tpiWeights, tpiIndexes);
}

void Vbm::performHyperspectralInterpolation(const long k, const long m, Context& context, const valarray<double>& surfaceReflectances, valarray<double>& hyperSpectralReflectances) {
    const valarray<double>& wavelengths = getAuxdataProvider(context, Constants::AUX_ID_VPRTAX).getVectorDouble("wavelength");
    for(size_t i = 0; i < wavelengths.size(); i++) {
        hyperSpectralReflectances[i] = linearInterpolation(k, m, surfaceReflectances, wavelengths[i]);
    }
}

double Vbm::linearInterpolation(long k, long m, const valarray<double>& surfaceReflectances, const double wavelength) {
    valarray<double> channelWavelengths(surfaceReflectances.size());
    for(size_t channel = 0; channel < surfaceReflectances.size(); channel++) {
        if(surfaceReflectances[channel] == Constants::FILL_VALUE_DOUBLE) {
            return Constants::FILL_VALUE_DOUBLE;
        }
        if(channel < 18) {
            const size_t index = olciInfoSegment->getGrid().getIndex(k, channel, m);
            channelWavelengths[channel] = olciInfoSegment->getAccessor("lambda0").getDouble(index);
        } else {
            channelWavelengths[channel] = getSlnWavelength(channel);
        }
    }

    return linearInterpolation(channelWavelengths, surfaceReflectances, wavelength);
}

double Vbm::linearInterpolation(const valarray<double> x, const valarray<double> f, const double wavelength) {
    size_t x0Index = numeric_limits<size_t>::max();
    size_t x1Index = numeric_limits<size_t>::max();
    double x0Delta = numeric_limits<size_t>::max();
    double x1Delta = numeric_limits<size_t>::max();
    for(size_t i = 0; i < x.size(); i++) {
        double delta = std::abs(x[i] - wavelength);
        if(x[i] <= wavelength) {
            if(delta < x0Delta) {
                x0Index = i;
                x0Delta = delta;
            }
        } else {
            if(delta < x1Delta) {
                x1Index = i;
                x1Delta = delta;
            }
        }
    }

    const double x0 = x[x0Index];
    const double x1 = x[x1Index];
    const double f0 = f[x0Index];
    const double f1 = f[x1Index];

    return f0 + (f1 - f0) / (x1 - x0) * (wavelength - x0);
}


double Vbm::getSlnWavelength(size_t channel) {
    switch (channel) {
    case 18:
        return 550;
    case 19:
        return 665;
    case 20:
        return 865;
    case 21:
        return 1375;
    case 22:
        return 1610;
    case 23:
        return 2250;
    }
    BOOST_THROW_EXCEPTION(logic_error("invalid channel index '" + lexical_cast<string>(channel) + "'"));
}

void Vbm::performHyperspectralUpscaling(const valarray<double>& hyperSpectralReflectances, const Pixel& p, valarray<double>& toaReflectances) {
    valarray<double> coordinates(7);

    valarray<double> f(vgtLutRhoAtm->getDimensionCount());
    valarray<double> w(vgtLutRhoAtm->getMatrixWorkspaceSize());

    valarray<double> rhoAtm(914);
    valarray<double> rAtm(914);
    valarray<double> tSun(914);
    valarray<double> tView(914);

    coordinates[0] = p.airPressure;
    coordinates[1] = p.waterVapour;
    coordinates[2] = p.aot;
    coordinates[3] = p.aerosolModel;
    vgtLutRhoAtm->getVector(&coordinates[0], rhoAtm, f, w);

    coordinates[0] = std::abs(p.vaaOlc - p.saa);
    coordinates[1] = p.sza;
    coordinates[2] = p.vzaOlc;
    coordinates[3] = p.airPressure;
    coordinates[4] = p.waterVapour;
    coordinates[5] = p.aot;
    coordinates[6] = p.aerosolModel;
    vgtLutRAtm->getVector(&coordinates[0], rAtm, f, w);

    coordinates[0] = p.sza;
    coordinates[1] = p.airPressure;
    coordinates[2] = p.waterVapour;
    coordinates[3] = p.aot;
    coordinates[4] = p.aerosolModel;
    vgtLutT->getVector(&coordinates[0], tSun, f, w);

    coordinates[0] = p.vzaOlc;
    vgtLutT->getVector(&coordinates[0], tView, f, w);

    for(size_t h = 0; h < hyperSpectralReflectances.size(); h++) {
        toaReflectances[h] = hyperspectralUpscale(p.sza, p.vzaOlc, p.ozone, hyperSpectralReflectances[h], (*vgtCo3)[h], rhoAtm[h], rAtm[h], tSun[h], tView[h]);
    }
}

double Vbm::hyperspectralUpscale(double sza, double vzaOlc, double ozone, double hyperSpectralReflectance, double co3, double rhoAtm, double rAtm, double tSun, double tView) {
    if(hyperSpectralReflectance == Constants::FILL_VALUE_DOUBLE) {
        return Constants::FILL_VALUE_DOUBLE;
    }
    double M = 0.5 * (1 / std::cos(sza) + 1 / (std::cos(vzaOlc)));
    double tO3 = std::exp(-M * ozone * co3);
    double g = tSun * tView;
    return tO3 * (rAtm + (g * hyperSpectralReflectance) / ((1 - rhoAtm) * hyperSpectralReflectance));
}

void Vbm::performHyperspectralFiltering(valarray<double>& toaReflectances, valarray<double>& filteredRToa) {
    valarray<double> coordinates(1);
    valarray<double> f(vgtBSrfLuts[0]->getDimensionCount());
    valarray<double> w(vgtBSrfLuts[0]->getMatrixWorkspaceSize());

    for(size_t b = 0; b < 4; b++) {
        double numerator = 0.0;
        double denominator = 0.0;
        for(size_t h = 0; h < 914; h++) {
            coordinates[0] = h;
            double solarIrr = vgtLutSolarIrradiance->getScalar(&coordinates[0], f, w);
            double bSurf = vgtBSrfLuts[b]->getScalar(&coordinates[0], f, w);
            numerator += solarIrr * bSurf * toaReflectances[h];
            denominator += solarIrr * bSurf;
        }
        if(denominator != 0.0) {
            filteredRToa[b] = numerator / denominator;
        } else {
            filteredRToa[b] = Constants::FILL_VALUE_DOUBLE;
        }
    }
}

uint8_t Vbm::getFlagsAndFills(Pixel& p, valarray<double>& vgtToaReflectances) {
    uint8_t flags = 0;
    if (p.radiances[1] != Constants::FILL_VALUE_DOUBLE && p.radiances[2] != Constants::FILL_VALUE_DOUBLE) {
        flags |= Constants::VGT_B0_GOOD;
    } else {
        vgtToaReflectances[0] = Constants::FILL_VALUE_DOUBLE;
    }
    bool vgtB2Good = true;
    for (size_t i = 5; i < 10; i++) {
        if (p.radiances[i] == Constants::FILL_VALUE_DOUBLE) {
            vgtB2Good = false;
            vgtToaReflectances[1] = Constants::FILL_VALUE_DOUBLE;
            break;
        }
    }

    flags |= vgtB2Good ? Constants::VGT_B2_GOOD : 0;
    bool vgtB3Good = true;
    for (size_t i = 13; i < 18; i++) {
        if (p.radiances[i] == Constants::FILL_VALUE_DOUBLE) {
            vgtB3Good = false;
            vgtToaReflectances[2] = Constants::FILL_VALUE_DOUBLE;
            break;
        }
    }

    flags |= vgtB3Good ? Constants::VGT_B3_GOOD : 0;
    if (p.radiances[22] != Constants::FILL_VALUE_DOUBLE && p.radiances[23] != Constants::FILL_VALUE_DOUBLE) {
        flags |= Constants::VGT_MIR_GOOD;
    } else {
        vgtToaReflectances[3] = Constants::FILL_VALUE_DOUBLE;
    }
    return flags;
}

void Vbm::cleanup(valarray<double>& surfaceReflectances, valarray<double>& hyperSpectralReflectances, valarray<double>& toaReflectances, valarray<double>& vgtToaReflectances) {
    surfaceReflectances.resize(surfaceReflectances.size(), 0.0);
    hyperSpectralReflectances.resize(hyperSpectralReflectances.size(), 0.0);
    toaReflectances.resize(toaReflectances.size(), 0.0);
    vgtToaReflectances.resize(vgtToaReflectances.size(), 0.0);
}

void Vbm::setValues(const size_t index, const uint8_t flags, const valarray<double>& vgtToaReflectances) {
    vgtFlagsAccessor->setUByte(index, flags);
    valarray<Accessor*> targetAccessors(4);
    targetAccessors[0] = vgtB0Accessor;
    targetAccessors[1] = vgtB2Accessor;
    targetAccessors[2] = vgtB3Accessor;
    targetAccessors[3] = vgtMirAccessor;

    for (size_t i = 0; i < targetAccessors.size(); i++) {
        const double value = vgtToaReflectances[i];
        if (value != Constants::FILL_VALUE_DOUBLE) {
            targetAccessors[i]->setDouble(index, value);
        } else {
            targetAccessors[i]->setFillValue(index);
        }
    }
}