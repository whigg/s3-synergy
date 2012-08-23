/*
 * Copyright (C) 2012  Brockmann Consult GmbH (info@brockmann-consult.de)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation. This program is distributed in the hope it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#include <algorithm>
#include <cmath>
#include <iostream>

#include "PixelFinder.h"

PixelFinder::PixelFinder(GeoLocation& geoLocation, double pixelSize) : geoLocation(geoLocation), pixelSize(pixelSize), tpi(0, geoLocation.getGrid().getSizeK()), tpIndices(valarray<double>(), geoLocation.getGrid().getSizeK()) {
	const Grid& grid = geoLocation.getGrid();

	const long sizeK = grid.getSizeK();
	const long sizeL = grid.getSizeL();
	const long sizeM = grid.getSizeM();

	size_t tpCount = computeTiePointCount(1, sizeL, sizeM);

	for (long k = 0; k < sizeK; k++) {
		tpIndices[k].resize(tpCount);
		valarray<double> tpLats(tpCount);
		valarray<double> tpLons(tpCount);
		for (long i = 0, l = 0; l < sizeL; l += 32) {
			for (long m = 0; m < sizeM; m += 32, i++) {
				const size_t index = grid.getIndex(k, l, m);
				tpLats[i] = geoLocation.getLat(index);
				tpLons[i] = geoLocation.getLon(index);
				tpIndices[k][i] = index;
			}
		}
		tpi[k] = new TiePointInterpolator<double>(tpLons, tpLats);
	}
}

PixelFinder::~PixelFinder() {
	for (size_t k = tpi.size(); k-- > 0;) {
		delete tpi[k];
	}
}

bool PixelFinder::findSourcePixel(double targetLat, double targetLon, long& resultK, long& resultL, long& resultM) const {
	using std::acos;
	using std::max;
	using std::min;

	const Grid& grid = geoLocation.getGrid();

	valarray<double> w(1);
	valarray<size_t> i(1);
	double delta = -1.0;

	for (long k = 0; grid.getSizeK(); k++) {
		tpi[k]->prepare(targetLon, targetLat, w, i);

		const size_t index = tpi[k]->interpolate(tpIndices[k], w, i);
		const long l = getL(index);
		const long m = getM(index);

		updateNearestPixel(targetLat, targetLon, k, l, m, resultK, resultL, resultM, delta);

		for (long b = 32; b > 0; b >>= 1) {
			const long midK = k;
			const long midL = l;
			const long midM = m;

			const long outerMinL = max(grid.getMinL(), midL - b);
			const long outerMaxL = min(grid.getMaxL(), midL + b);
			const long outerMinM = max(grid.getMinM(), midM - b);
			const long outerMaxM = min(grid.getMaxM(), midM + b);

			if (true) { // consider outer points in the N, S, E, and W
				updateNearestPixel(targetLat, targetLon, k, midL, outerMinM, resultK, resultL, resultM, delta);
				updateNearestPixel(targetLat, targetLon, k, midL, outerMaxM, resultK, resultL, resultM, delta);
				updateNearestPixel(targetLat, targetLon, k, outerMaxL, midM, resultK, resultL, resultM, delta);
				updateNearestPixel(targetLat, targetLon, k, outerMinL, midM, resultK, resultL, resultM, delta);
			}
			if (true) { // consider outer points in the NW, SW, SE, and NE
				updateNearestPixel(targetLat, targetLon, k, outerMinL, outerMinM, resultK, resultL, resultM, delta);
				updateNearestPixel(targetLat, targetLon, k, outerMaxL, outerMinM, resultK, resultL, resultM, delta);
				updateNearestPixel(targetLat, targetLon, k, outerMaxL, outerMaxM, resultK, resultL, resultM, delta);
				updateNearestPixel(targetLat, targetLon, k, outerMinL, outerMaxM, resultK, resultL, resultM, delta);
			}
			if (true) { // consider inner points in the NW, SW, SE, and NE
				const long innerMinL = max(outerMinL, midL - b / 2);
				const long innerMaxL = min(outerMaxL, midL + b / 2);
				const long innerMinM = max(outerMinM, midM - b / 2);
				const long innerMaxM = min(outerMaxM, midM + b / 2);

				updateNearestPixel(targetLat, targetLon, k, innerMinL, innerMinM, resultK, resultL, resultM, delta);
				updateNearestPixel(targetLat, targetLon, k, innerMaxL, innerMinM, resultK, resultL, resultM, delta);
				updateNearestPixel(targetLat, targetLon, k, innerMaxL, innerMaxM, resultK, resultL, resultM, delta);
				updateNearestPixel(targetLat, targetLon, k, innerMinL, innerMaxM, resultK, resultL, resultM, delta);
			}
		}
	}

    return acos(delta) * DEG < 0.71 * pixelSize;
}

void PixelFinder::updateNearestPixel(double targetLat, double targetLon, long k, long l, long m, long& resultK, long& resultL, long& resultM, double& maxDelta) const {
	const size_t sourceIndex = geoLocation.getGrid().getIndex(k, l, m);
	const double sourceLat = geoLocation.getLat(sourceIndex);
	const double sourceLon = geoLocation.getLon(sourceIndex);
	const double delta = TiePointInterpolator<double>::cosineDistance(targetLon, targetLat, sourceLon, sourceLat);

	if (delta > maxDelta) {
		resultK = k;
		resultL = l;
		resultM = m;
		maxDelta = delta;
	}
}

size_t PixelFinder::computeTiePointCount(long sizeK, long sizeL, long sizeM) const {
    size_t tpCount = 0;
    for (long k = 0; k < sizeK; k++) {
        for (long l = 0; l < sizeL; l += 32) {
            for (long m = 0; m < sizeM; m += 32) {
                tpCount++;
            }
        }
    }
    return tpCount;
}
