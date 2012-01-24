/*
 * Vpr.h
 *
 *  Created on: Dec 01, 2011
 *      Author: thomasstorm
 */

#ifndef VPR_H_
#define VPR_H_

#include "../core/Pixel.h"
#include "../modules/BasicModule.h"
#include "../util/PixelFinder.h"

class Vpr : public BasicModule, private GeoLocation {
public:
	Vpr();
	virtual ~Vpr();

	void start(Context& context);
	void process(Context& context);
	void process2(Context& context);

private:
	friend class VprTest;

	double getLat(size_t index) const {
		return latAccessor->getDouble(index);
	}

	double getLon(size_t index) const {
		return lonAccessor->getDouble(index);
	}

	const Grid& getGrid() const {
		return geoSegment->getGrid();
	}

	void setupAccessors(Context& context);
	void getMinMaxSourceLat(double& minLat, double& maxLat) const;
	void getMinMaxTargetLat(double& minLat, double& maxLat, long firstL, long lastL) const;
	void getMinMaxSourceLon(double& minLon, double& maxLon) const;

	double getTargetLat(long l) const ;
	double getTargetLon(long m) const;

	double getSubsampledTargetLat(long l) const ;
	double getSubsampledTargetLon(long m) const;

	int maxTargetLat;
	int minTargetLat;
	int maxTargetLon;
	int minTargetLon;

	const Segment* synSegment;
	const Segment* geoSegment;

	const Accessor* latAccessor;
	const Accessor* lonAccessor;

    valarray<Accessor*> sourceReflectanceAccessors;
    valarray<Accessor*> targetReflectanceAccessors;
    Accessor* targetFlagsAccessor;
    Accessor* sourceFlagsAccessor;

    static const int TARGET_PIXELS_PER_DEGREE = 112;
    static const int SUBSAMPLED_TARGET_PIXELS_PER_DEGREE = 14;
	static const double DEGREES_PER_TARGET_PIXEL = 1.0 / TARGET_PIXELS_PER_DEGREE;
	static const double DEGREES_PER_SUBSAMPLED_TARGET_PIXEL = 1.0 / SUBSAMPLED_TARGET_PIXELS_PER_DEGREE;
};

#endif /* VPR_H_ */
