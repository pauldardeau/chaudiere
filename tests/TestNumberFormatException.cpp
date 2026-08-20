// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <string.h>

#include "TestNumberFormatException.h"
#include "NumberFormatException.h"

using namespace chaudiere;

//******************************************************************************

TestNumberFormatException::TestNumberFormatException() :
   poivre::TestSuite("TestNumberFormatException") {
}

//******************************************************************************

void TestNumberFormatException::runTests() {
   testDefaultConstructor();
   testConstructorWithMessage();
   testCopyConstructor();
   testAssignmentOperator();
   testGetType();
}

//******************************************************************************

void TestNumberFormatException::testDefaultConstructor() {
   TEST_CASE("testDefaultConstructor");

   NumberFormatException nfe;
   require(nfe.whatString().empty(), "default constructor should result in an empty message");
}

//******************************************************************************

void TestNumberFormatException::testConstructorWithMessage() {
   TEST_CASE("testConstructorWithMessage");

   NumberFormatException nfe("not a number: abc");
   requireStringEquals("not a number: abc", nfe.whatString(), "message should match constructor argument");
}

//******************************************************************************

void TestNumberFormatException::testCopyConstructor() {
   TEST_CASE("testCopyConstructor");

   NumberFormatException nfe("bad value");
   NumberFormatException nfeCopy(nfe);
   requireStringEquals("bad value", nfeCopy.whatString(), "copy should have the same message");
}

//******************************************************************************

void TestNumberFormatException::testAssignmentOperator() {
   TEST_CASE("testAssignmentOperator");

   NumberFormatException source("source message");
   NumberFormatException target("target message");
   target = source;
   requireStringEquals("source message", target.whatString(), "target should have the same message after assignment");
}

//******************************************************************************

void TestNumberFormatException::testGetType() {
   TEST_CASE("testGetType");

   NumberFormatException nfe("x");
   require(0 == ::strcmp("NumberFormatException", nfe.getType()), "getType should return the exception's class name");
}

//******************************************************************************
