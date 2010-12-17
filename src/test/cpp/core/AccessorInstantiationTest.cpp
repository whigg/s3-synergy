/* 
 * Copyright (C) 2010 by Brockmann Consult (info@brockmann-consult.de)
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
 *
 * File:   AccessorInstantiationTest.cpp
 * Author: ralf
 * 
 * Created on December 17, 2010, 5:17 PM
 */

#include "AccessorInstantiationTest.h"
#include "../../../main/cpp/core/ByteAccessor.h"
#include "../../../main/cpp/core/DoubleAccessor.h"
#include "../../../main/cpp/core/FloatAccessor.h"
#include "../../../main/cpp/core/IntAccessor.h"
#include "../../../main/cpp/core/LongAccessor.h"
#include "../../../main/cpp/core/ShortAccessor.h"
#include "../../../main/cpp/core/GridImpl.h"

CPPUNIT_TEST_SUITE_REGISTRATION(AccessorInstantiationTest);

AccessorInstantiationTest::AccessorInstantiationTest() {
}

AccessorInstantiationTest::~AccessorInstantiationTest() {
}

void AccessorInstantiationTest::setUp() {
    accessor = 0;
}

void AccessorInstantiationTest::tearDown() {
    if (accessor != 0) {
        delete accessor;
    }
}

void AccessorInstantiationTest::testInstantiateByteAccessor() {
    accessor = new ByteAccessor(GridImpl(1, 1, 1));
    CPPUNIT_ASSERT(accessor != 0);
    CPPUNIT_ASSERT_NO_THROW(accessor->getByteData());
}

void AccessorInstantiationTest::testInstantiateDoubleAccessor() {
    accessor = new DoubleAccessor(GridImpl(1, 1, 1));
    CPPUNIT_ASSERT(accessor != 0);
    CPPUNIT_ASSERT_NO_THROW(accessor->getDoubleData());
}

void AccessorInstantiationTest::testInstantiateFloatAccessor() {
    accessor = new FloatAccessor(GridImpl(1, 1, 1));
    CPPUNIT_ASSERT(accessor != 0);
    CPPUNIT_ASSERT_NO_THROW(accessor->getFloatData());
}

void AccessorInstantiationTest::testInstantiateIntAccessor() {
    accessor = new IntAccessor(GridImpl(1, 1, 1));
    CPPUNIT_ASSERT(accessor != 0);
    CPPUNIT_ASSERT_NO_THROW(accessor->getIntData());
}

void AccessorInstantiationTest::testInstantiateLongAccessor() {
    accessor = new LongAccessor(GridImpl(1, 1, 1));
    CPPUNIT_ASSERT(accessor != 0);
    CPPUNIT_ASSERT_NO_THROW(accessor->getLongData());
}

void AccessorInstantiationTest::testInstantiateShortAccessor() {
    accessor = new ShortAccessor(GridImpl(1, 1, 1));
    CPPUNIT_ASSERT(accessor != 0);
    CPPUNIT_ASSERT_NO_THROW(accessor->getShortData());
}
