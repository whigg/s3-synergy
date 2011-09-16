/* 
 * File:   ColTest.cpp
 * Author: thomasstorm
 * 
 * Created on Sep 15, 2011, 3:30 PM
 */

#include <cstdlib>

#include "../../../../src/main/cpp/core/Processor.h"
#include "../../../../src/main/cpp/reader/SynL1Reader.h"
#include "../../../../src/main/cpp/modules/Col.h"
#include "../../../../src/main/cpp/writer/SynL2Writer.h"
#include "../../../../src/main/cpp/util/DictionaryParser.h"
#include "../../../../src/main/cpp/util/JobOrderParser.h"

#include "ColTest.h"

extern shared_ptr<Context> context;

using std::getenv;

CPPUNIT_TEST_SUITE_REGISTRATION(ColTest);

ColTest::ColTest() {
}

ColTest::~ColTest() {
}

void ColTest::setUp() {
    XPathInitializer init;


    const string S3_SYNERGY_HOME = getenv("S3_SYNERGY_HOME");
    shared_ptr<Dictionary> dictionary = DictionaryParser().parse(
            S3_SYNERGY_HOME + "/src/main/resources/dictionary");
    shared_ptr<JobOrder> jobOrder = JobOrderParser().parse(
            S3_SYNERGY_HOME
                    + "/src/test/resources/jobs/JobOrder.SY_UNT_ACO.xml");
    shared_ptr<Module> reader = shared_ptr<Module>(new SynL1Reader());
    shared_ptr<Module> col = shared_ptr<Module>(new Col());
    shared_ptr<Module> writer = shared_ptr<Module>(new SynL2Writer());

    context->setDictionary(dictionary);
    context->setJobOrder(jobOrder);
    context->addModule(reader);
    context->addModule(col);
    context->addModule(writer);
}

void ColTest::tearDown() {
    foreach(string id, context->getSegmentIds()) {
        context->removeSegment(id);
    }
    foreach(shared_ptr<Module> module, context->getModules()) {
        context->removeModule(module);
    }
}

void ColTest::testAddSlstrVariables() {
    Segment& segment = context->addSegment(Constants::SEGMENT_SYN_COLLOCATED, 10, 10, 5, 0, 9);

	// setting dummy type; this is done by reader normally, but not in test
	foreach(VariableDescriptor* vd, context->getDictionary()->getProductDescriptor("SY1").getVariableDescriptors()) {
		vd->setType(7);
	}

    CPPUNIT_ASSERT(!segment.hasVariable("L_1"));
    CPPUNIT_ASSERT(!segment.hasVariable("L_18"));
    CPPUNIT_ASSERT(!segment.hasVariable("L_19"));
    CPPUNIT_ASSERT(!segment.hasVariable("L_30"));
    CPPUNIT_ASSERT(!segment.hasVariable("L_25_exception"));
    CPPUNIT_ASSERT(!segment.hasVariable("L_30_exception"));

	Col col;
	col.addSlstrVariables(*context, segment);
	CPPUNIT_ASSERT(!segment.hasVariable("L_1"));
	CPPUNIT_ASSERT(!segment.hasVariable("L_18"));
	CPPUNIT_ASSERT(segment.hasVariable("L_19"));
	CPPUNIT_ASSERT(segment.hasVariable("L_30"));
	CPPUNIT_ASSERT(segment.hasVariable("L_25_exception"));
	CPPUNIT_ASSERT(segment.hasVariable("L_30_exception"));
}

void ColTest::testAddOlciVariables() {
    Segment& segment = context->addSegment(Constants::SEGMENT_SYN_COLLOCATED, 10, 10, 5, 0, 9);
	Segment& olciSegment = context->addSegment(Constants::SEGMENT_OLC, 10, 10, 5, 0, 9);
	// setting dummy type; this is done by reader normally, but not in test
	foreach(VariableDescriptor* vd, context->getDictionary()->getProductDescriptor("SY1").getVariableDescriptors()) {
		vd->setType(7);
	}

	olciSegment.addVariable("L_17", Constants::TYPE_BYTE);
	olciSegment.addVariable("L_17_er", Constants::TYPE_BYTE);
	olciSegment.addVariable("longitude", Constants::TYPE_BYTE);
	olciSegment.addVariable("latitude", Constants::TYPE_BYTE);
	olciSegment.addVariable("altitude", Constants::TYPE_BYTE);

    CPPUNIT_ASSERT(!segment.hasVariable("L_1"));
    CPPUNIT_ASSERT(!segment.hasVariable("L_18"));
    CPPUNIT_ASSERT(!segment.hasVariable("L_1_er"));
    CPPUNIT_ASSERT(!segment.hasVariable("L_18_er"));

	Col col;
	col.addOlciVariables(*context, segment, olciSegment);
	CPPUNIT_ASSERT(segment.hasVariable("L_1"));
	CPPUNIT_ASSERT(segment.hasVariable("L_18"));
	CPPUNIT_ASSERT(segment.hasVariable("L_1_er"));
	CPPUNIT_ASSERT(segment.hasVariable("L_18_er"));
}

void ColTest::testRetrievePositionVariableName() {
    Col collocation;
    string varName = "SLN_confidence";
    string variableName = collocation.retrievePositionVariableName(varName, Col::AXIS_COL);
    CPPUNIT_ASSERT(variableName.compare("x_corr_1") == 0);

    varName = "L_19";
    variableName = collocation.retrievePositionVariableName(varName, Col::AXIS_COL);
    CPPUNIT_ASSERT(variableName.compare("x_corr_1") == 0);

    varName = "L_20";
    variableName = collocation.retrievePositionVariableName(varName, Col::AXIS_COL);
    CPPUNIT_ASSERT(variableName.compare("x_corr_2") == 0);

    varName = "L_19";
    variableName = collocation.retrievePositionVariableName(varName, Col::AXIS_ROW);
    CPPUNIT_ASSERT(variableName.compare("y_corr_1") == 0);

    varName = "L_20";
    variableName = collocation.retrievePositionVariableName(varName, Col::AXIS_ROW);
    CPPUNIT_ASSERT(variableName.compare("y_corr_2") == 0);

    varName = "L_24";
    variableName = collocation.retrievePositionVariableName(varName, Col::AXIS_COL);
    CPPUNIT_ASSERT(variableName.compare("x_corr_6") == 0);

    varName = "L_24";
    variableName = collocation.retrievePositionVariableName(varName, Col::AXIS_ROW);
    CPPUNIT_ASSERT(variableName.compare("y_corr_6") == 0);

    varName = "L_19_er";
    variableName = collocation.retrievePositionVariableName(varName, Col::AXIS_COL);
    CPPUNIT_ASSERT(variableName.compare("x_corr_1") == 0);

    varName = "L_24_er";
    variableName = collocation.retrievePositionVariableName(varName, Col::AXIS_ROW);
    CPPUNIT_ASSERT(variableName.compare("y_corr_6") == 0);

    varName = "L_18";
    CPPUNIT_ASSERT_THROW(collocation.retrievePositionVariableName(varName, Col::AXIS_COL), logic_error);

    varName = "L_25";
    CPPUNIT_ASSERT_THROW(collocation.retrievePositionVariableName(varName, Col::AXIS_COL), logic_error);
}

void ColTest::testCol() {
	Processor processor;
	processor.process(*context);
}
