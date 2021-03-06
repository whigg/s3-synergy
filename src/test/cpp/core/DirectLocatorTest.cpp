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

#include "../../../main/cpp/core/DirectLocator.h"
#include "../../../main/cpp/core/DoubleAccessor.h"

#include "DirectLocatorTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION(DirectLocatorTest);

DirectLocatorTest::DirectLocatorTest() {
}

DirectLocatorTest::~DirectLocatorTest() {
}

void DirectLocatorTest::setUp() {
}

void DirectLocatorTest::tearDown() {
}

void DirectLocatorTest::testRotation_Identity() {
	const Rotation r;

	double lat;
	double lon;

	lat = 0.0;
	lon = 0.0;
	r.transform(lat, lon);

	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, lat, 0.0);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, lon, 0.0);

	r.inverseTransform(lat, lon);

	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, lat, 0.0);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, lon, 0.0);

	lat = 7.0;
	lon = 2.0;
	r.transform(lat, lon);

	CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0, lat, 1.0E-15);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, lon, 1.0E-15);

	r.inverseTransform(lat, lon);

	CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0, lat, 1.0E-15);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, lon, 1.0E-15);
}

void DirectLocatorTest::testRotation_Antipode() {
	const Rotation r(0.0, 180.0);

	double lat;
	double lon;

	lat = 0.0;
	lon = 0.0;
	r.transform(lat, lon);

	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, lat, 1.0E-12);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(-180.0, lon, 1.0E-12);

	r.inverseTransform(lat, lon);

	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, lat, 1.0E-12);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, lon, 1.0E-12);

	lat = 7.0;
	lon = -180.0;
	r.transform(lat, lon);

	CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0, lat, 1.0E-12);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, lon, 1.0E-12);

	r.inverseTransform(lat, lon);

	CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0, lat, 1.0E-12);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(-180.0, lon, 1.0E-12);
}

void DirectLocatorTest::testLocation() {
	class TestGrid : public virtual Grid {
	public:
		TestGrid() {
		}

		~TestGrid() {
		}

		long getMinInMemoryL() const {
			return 0;
		}

		long getMaxInMemoryL() const {
			return 2;
		}

		long getSizeK() const {
			return 1;
		}

		long getSizeL() const {
			return 3;
		}

		long getSizeM() const {
			return 3;
		}

		long getSize() const {
			return getSizeK() * getSizeL() * getSizeM();
		}

		long getStrideK() const {
			return getSizeL() * getSizeM();
		}

		long getStrideL() const {
			return getSizeM();
		}

		long getStrideM() const {
			return 1;
		}

		size_t getIndex(long k, long l, long m) const throw (out_of_range) {
			return k * getStrideK() + l * getStrideL() + m * getStrideM();
		}

		long getMinK() const {
			return 0;
		}

		long getMaxK() const {
			return getSizeK() - 1;
		}

		long getMinL() const {
			return getMinInMemoryL();
		}

		long getMaxL() const {
			return getMinInMemoryL() + getSizeL() - 1;
		}

		long getMinM() const {
			return 0;
		}

		long getMaxM() const {
			return getSizeM() - 1;
		}

		bool isValidPosition(long k, long l, long m) const {
			return true;
		}

		long getK(size_t i) const {
		    return 0;
		}

		long getM(size_t i) const {
		    return 0;
		}

		long getL(size_t i) const {
		    return 0;
		}
	};

	TestGrid grid;
	DoubleAccessor latAccessor(9);
	DoubleAccessor lonAccessor(9);

	latAccessor.setDouble(0, 2.0);
	latAccessor.setDouble(1, 2.0);
	latAccessor.setDouble(2, 2.0);
	latAccessor.setDouble(3, 1.0);
	latAccessor.setDouble(4, 1.0);
	latAccessor.setDouble(5, 1.0);
	latAccessor.setDouble(6, 0.0);
	latAccessor.setDouble(7, 0.0);
	latAccessor.setDouble(8, 0.0);

	lonAccessor.setDouble(0, 0.0);
	lonAccessor.setDouble(1, 1.0);
	lonAccessor.setDouble(2, 2.0);
	lonAccessor.setDouble(3, 0.0);
	lonAccessor.setDouble(4, 1.0);
	lonAccessor.setDouble(5, 2.0);
	lonAccessor.setDouble(6, 0.0);
	lonAccessor.setDouble(7, 1.0);
	lonAccessor.setDouble(8, 2.0);

	DirectLocator dl(latAccessor, lonAccessor, grid);

	double lat;
	double lon;

	dl.getLocation(0, 0.5, 0.5, lat, lon);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, lat, 1.0E-15);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, lon, 1.0E-15);

	dl.getLocation(0, 1.5, 0.5, lat, lon);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, lat, 1.0E-15);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, lon, 1.0E-15);

	dl.getLocation(0, 2.5, 0.5, lat, lon);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, lat, 1.0E-15);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, lon, 1.0E-15);

	dl.getLocation(0, 0.5, 1.5, lat, lon);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, lat, 1.0E-15);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, lon, 1.0E-15);

	dl.getLocation(0, 1.5, 1.5, lat, lon);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, lat, 1.0E-15);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, lon, 1.0E-15);

	dl.getLocation(0, 2.5, 1.5, lat, lon);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, lat, 1.0E-15);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, lon, 1.0E-15);

	dl.getLocation(0, 0.5, 2.5, lat, lon);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, lat, 1.0E-15);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, lon, 1.0E-15);

	dl.getLocation(0, 1.5, 2.5, lat, lon);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, lat, 1.0E-15);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, lon, 1.0E-15);

	dl.getLocation(0, 2.5, 2.5, lat, lon);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, lat, 1.0E-15);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, lon, 1.0E-15);

	dl.getLocation(0, 1.0, 1.0, lat, lon);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1.5, lat, 1.0E-4);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, lon, 1.0E-4);

	dl.getLocation(0, 2.0, 1.0, lat, lon);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1.5, lat, 1.0E-4);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1.5, lon, 1.0E-4);

	dl.getLocation(0, 1.0, 2.0, lat, lon);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, lat, 1.0E-4);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, lon, 1.0E-4);

	dl.getLocation(0, 2.0, 2.0, lat, lon);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, lat, 1.0E-4);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1.5, lon, 1.0E-4);
}

