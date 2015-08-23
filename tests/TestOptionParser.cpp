// Copyright Paul Dardeau, SwampBits LLC 2015
// BSD License

#include "TestOptionParser.h"
#include "Thread.h"

using namespace chaudiere;

//******************************************************************************

TestOptionParser::TestOptionParser() :
   TestSuite("TestOptionParser") {
}

//******************************************************************************

void TestOptionParser::runTests() {
   testConstructor();
   testAddBooleanOption();
   testAddOption();
   testHasOption();
   testGetOptionValue();
   testHasBooleanOption();
   testParseArgs();
}

//******************************************************************************

void TestOptionParser::testConstructor() {
   TEST_CASE("testConstructor");
   //TODO: implement testConstructor
}

//******************************************************************************

void TestOptionParser::testAddBooleanOption() {
   TEST_CASE("testAddBooleanOption");
   //TODO: testAddBooleanOption
}

//******************************************************************************

void TestOptionParser::testAddOption() {
   TEST_CASE("testAddOption");
   //TODO: implement testAddOption
}

//******************************************************************************

void TestOptionParser::testHasOption() {
   TEST_CASE("testHasOption");
   //TODO: implement testHasOption
}

//******************************************************************************

void TestOptionParser::testGetOptionValue() {
   TEST_CASE("testGetOptionValue");
   //TODO: implement testOptionValue
}

//******************************************************************************

void TestOptionParser::testHasBooleanOption() {
   TEST_CASE("testHasBooleanOption");
   //TODO: implement testHasBooleanOption
}

//******************************************************************************

void TestOptionParser::testParseArgs() {
   TEST_CASE("testParseArgs");
   //TODO: implement testParseArgs
}

//******************************************************************************
