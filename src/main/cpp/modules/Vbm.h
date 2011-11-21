/*
 * Vbm.h
 *
 *  Created on: Nov 17, 2011
 *      Author: thomasstorm
 */

#ifndef VBM_H_
#define VBM_H_

#include <math.h>

#include "../modules/BasicModule.h"
#include "../core/Pixel.h"
#include "../core/TiePointInterpolator.h"

class Vbm: public BasicModule {
public:
	Vbm();
	virtual ~Vbm();

	void start(Context& context);
	void stop(Context& context);
	void process(Context& context);

	static double computeT550(double lat) {
	    return 0.2 * (std::cos(lat) - 0.25) * cube(std::sin(lat + M_PI_2)) + 0.05;
	}

private:
	friend class VbmTest;

	template<class T>
	static T cube(T x) {
	    return x * x * x;
	}

    template<class T>
    static void copy(const std::valarray<T>& s, std::valarray<T>& t) {
        t.resize(s.size());
        std::copy(&s[0], &s[s.size()], &t[0]);
    }

	uint16_t amin;
	Segment* collocatedSegment;
	LookupTable<double>* synLutRhoAtm;
	LookupTable<double>* synLutOlcRatm;
	LookupTable<double>* synLutSlnRatm;
	LookupTable<double>* synLutT;
	const valarray<double>* synCo3;

	LookupTable<double>* vgtLutRhoAtm;
	LookupTable<double>* vgtLutRAtm;
	LookupTable<double>* vgtLutT;
	LookupTable<double>* vgtLutSolarIrradiance;
	valarray<LookupTable<double>*> vgtBSrfLuts;
	const valarray<double>* vgtCo3;

    Accessor* synOzoneAccessor;
    Accessor* synLatitudeAccessor;
    Accessor* synLongitudeAccessor;

    valarray<Accessor*> synRadianceAccessors;
    valarray<Accessor*> synSolarIrradianceAccessors;

    Accessor* vgtFlagsAccessor;
    Accessor* vgtB0Accessor;
    Accessor* vgtB2Accessor;
    Accessor* vgtB3Accessor;
    Accessor* vgtMirAccessor;

    shared_ptr<TiePointInterpolator<double> > tiePointInterpolatorOlc;
    shared_ptr<TiePointInterpolator<double> > tiePointInterpolatorSln;

    valarray<double>* szaOlcTiePoints;
    valarray<double>* saaOlcTiePoints;
    valarray<double>* vzaOlcTiePoints;
    valarray<double>* vzaSlnTiePoints;

    valarray<double>* waterVapourTiePoints;
    valarray<double>* airPressureTiePoints;
    valarray<double>* ozoneTiePoints;

    // todo - sort
	static double surfaceReflectance(double ozone, double vza, double sza, double solarIrradiance, double radiance,
	        double co3, double rhoAtm, double rAtm, double tSun, double tView);

    void addVariables();
	void downscale(const Pixel& p, valarray<double>& surfReflNadirSyn);
	void performHyperspectralInterpolation(Context& context, const valarray<double>& surfaceReflectances, valarray<double>& hyperSpectralReflectances);
	double linearInterpolation(const valarray<double>& surfaceReflectances, double wavelength);
	void performHyperspectralUpscaling(const valarray<double>& hyperSpectralReflectances, const Pixel& p, valarray<double>& toaReflectances);
	double hyperspectralUpscale(double sza, double vzaOlc, double ozone, double hyperSpectralReflectance, double co3, double rhoAtm, double rAtm, double tSun, double tView);
	void performHyperspectralFiltering(valarray<double>& toaReflectances, valarray<double>& filteredRToa);
	uint8_t getFlagsAndFills(Pixel& p, valarray<double>& vgtToaReflectances);
	void cleanup(valarray<double>& surfaceReflectances, valarray<double>& hyperSpectralReflectances, valarray<double>& toaReflectances, valarray<double>& vgtToaReflectances);
	void setupPixel(Pixel& p, size_t index);
	void prepareAuxdata(Context& context);
	void prepareTiePointData(Context& context);
	void setValues(const size_t index, const uint8_t flags, const valarray<double>& vgtToaReflectances);
};

#endif /* VBM_H_ */
