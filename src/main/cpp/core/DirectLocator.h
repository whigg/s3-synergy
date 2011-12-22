/*
 * DirectLocator.h
 *
 *  Created on: Dec 20, 2011
 *      Author: ralf
 */

#ifndef DIRECTLOCATOR_H_
#define DIRECTLOCATOR_H_

#include "Accessor.h"
#include "Grid.h"

class Rotation {
public:
	Rotation(double lat = 0.0, double lon = 0.0);

	~Rotation();

	void transform(double& lat, double& lon) const;
	void inverseTransform(double& lat, double& lon) const;

private:
	static double toDegrees(double r) {
		return r * (180.0 / 3.14159265358979323846);
	}

	static double toRadians(double d) {
		return d * (3.14159265358979323846 / 180.0);
	}

	double a11;
	double a12;
	double a13;
	double a21;
	double a22;
	double a23;
	double a31;
	double a32;
	double a33;
};

class DirectLocator {
public:
	DirectLocator(const Accessor& latAccessor, const Accessor& lonAccessor, const Grid& grid);
	~DirectLocator();

	bool getLocation(const long k, const double x, const double y, double& lat, double& lon) const;

private:
	void interpolate(const long k, const long y0, const long x0, const double wx, const double wy, double& lat, double& lon) const;

	static double interpolate(const double wi, const double wj, const double x00, const double x10, const double x01, const double x11) {
		return x00 + wi * (x10 - x00) + wj * ((x01 - x00) + wi * (x11 - x10 - x01 + x00));
	}

	const Accessor& latAccessor;
	const Accessor& lonAccessor;
	const Grid& grid;
};

#endif /* DIRECTLOCATOR_H_ */