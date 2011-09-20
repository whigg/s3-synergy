/*
 * TiePointInterpolator.h
 *
 *  Created on: Aug 25, 2011
 *      Author: ralf
 */

#ifndef TIEPOINTINTERPOLATOR_H_
#define TIEPOINTINTERPOLATOR_H_

#include <algorithm>
#include <cassert>
#include <cmath>
#include <valarray>

using std::valarray;

/**
 * Interpolates between a set of (lon, lat) tie points by means of inverse
 * distance weighting.
 *
 * The current implementation uses the reciprocal arc distance, which is the
 * most precise, but also the most costly weight.
 *
 * @author Ralf Quast
 */
template<class W>
class TiePointInterpolator {
public:
	TiePointInterpolator(const valarray<W>& tpLons, const valarray<W>& tpLats);
	~TiePointInterpolator();

	void prepare(W lon, W lat, valarray<W>& weights, valarray<size_t>& indexes) const;
	W interpolate(const valarray<W>& field, const valarray<W>& weights, const valarray<size_t>& indexes) const;

private:
	class TiePointIndexComparator {
	public:
		TiePointIndexComparator(const valarray<W>& tpLons, const valarray<W>& tpLats) :
				tpLons(tpLons), tpLats(tpLats) {
		}

		~TiePointIndexComparator() {
		}

		bool operator()(const size_t i1, const size_t i2) const {
			return tpLats[i1] < tpLats[i2];
		}
	private:
		const valarray<W>& tpLons;
		const valarray<W>& tpLats;
	};

	void reorder(valarray<W>& array, const valarray<size_t>& reordering) const;
	W cosineDistance(W lon, W lat, size_t i) const;

	valarray<W> tpLons;
	valarray<W> tpLats;
	valarray<size_t> ordering;

	static const W RAD = W(3.14159265358979323846) / W(180.0);
};

template<class W>
TiePointInterpolator<W>::TiePointInterpolator(const valarray<W>& tpLons, const valarray<W>& tpLats) :
		tpLons(tpLons), tpLats(tpLats), ordering(tpLats.size()) {
	using std::sort;

	for (size_t i = 0; i < ordering.size(); i++) {
		ordering[i] = i;
	}
	sort(&ordering[0], &ordering[ordering.size()], TiePointIndexComparator(this->tpLons, this->tpLats));
	reorder(this->tpLons, ordering);
	reorder(this->tpLats, ordering);
}

template<class W>
TiePointInterpolator<W>::~TiePointInterpolator() {
}

template<class W>
void TiePointInterpolator<W>::reorder(valarray<W>& array, const valarray<size_t>& reordering) const {
	using std::copy;

	const valarray<W> reorderedArray = array[reordering];
	copy(&reorderedArray[0], &reorderedArray[reorderedArray.size()], &array[0]);
}

template<class W>
void TiePointInterpolator<W>::prepare(W lon, W lat, valarray<W>& weights, valarray<size_t>& indexes) const {
	using std::abs;
	using std::acos;
	using std::fill;
	using std::lower_bound;

	assert(weights.size() == indexes.size());

	const size_t n = weights.size();
	const size_t range = 150;
	const size_t midIndex = lower_bound(&tpLats[0], &tpLats[tpLats.size()], lat) - &tpLats[0];
	const size_t minIndex = midIndex >= range ? midIndex - range : 0;
	const size_t maxIndex = midIndex <= tpLats.size() - range ? midIndex + range : tpLats.size();
	fill(&weights[0], &weights[n], W(-1));

	for (size_t i = minIndex; i < maxIndex; i++) {
		const W d = cosineDistance(lon, lat, i);
		for (size_t k = 0; k < n; k++) {
			if (d > weights[k]) {
				for (size_t l = n - 1; l > k; l--) {
					weights[l] = weights[l - 1];
					indexes[l] = indexes[l - 1];
				}
				weights[k] = d;
				indexes[k] = ordering[i];
				break;
			}
		}
	}

	W sum = W(0.0);
	for (size_t i = 0; i < n; i++) {
		const W d = abs(acos(weights[i])); // arc distance
		if (d == W(0.0)) {
			for (size_t k = 0; k < n; k++) {
				weights[k] = k != i ? W(0.0) : W(1.0);
			}
			return;
		}
		weights[i] = W(1.0) / d;
		sum += weights[i];
	}
	for (size_t i = 0; i < n; i++) {
		weights[i] /= sum;
	}
}

template<class W>
W TiePointInterpolator<W>::interpolate(const valarray<W>& field, const valarray<W>& weights, const valarray<size_t>& indexes) const {
	assert(weights.size() == indexes.size());

	const size_t n = weights.size();

	W v = W(0);
	for (size_t i = 0; i < n; i++) {
		v += weights[i] * field[indexes[i]];
	}
	return v;
}

template<class W>
inline W TiePointInterpolator<W>::cosineDistance(W lon, W lat, size_t i) const {
	using std::cos;
	using std::sin;
	// http://www.movable-type.co.uk/scripts/latlong.html
	return sin(lat * RAD) * sin(tpLats[i] * RAD) + cos(lat * RAD) * cos(tpLats[i] * RAD) * cos((lon - tpLons[i]) * RAD);
}

#endif /* TIEPOINTINTERPOLATOR_H_ */