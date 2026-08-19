// Copyright Paul Dardeau, SwampBits LLC 2015
// BSD License

#include "TestSystemInfo.h"
#include "SystemInfo.h"

using namespace chaudiere;

//******************************************************************************

TestSystemInfo::TestSystemInfo() :
   poivre::TestSuite("TestSystemInfo") {
}

//******************************************************************************

void TestSystemInfo::runTests() {
   testConstructor();
   testCopyConstructor();
   testAssignmentOperator();
   testSysName();
   testNodeName();
   testRelease();
   testVersion();
   testMachine();
   testRetrievedSystemInfo();
}

//******************************************************************************

void TestSystemInfo::testConstructor() {
   TEST_CASE("testConstructor");

   SystemInfo si;
   // uname() should succeed on any real Linux/Unix host this suite runs on
   require(si.retrievedSystemInfo(), "constructing SystemInfo should successfully retrieve system information");
   requireNonEmptyString(si.sysName(), "sysName should be non-empty after a successful retrieval");
}

//******************************************************************************

void TestSystemInfo::testCopyConstructor() {
   TEST_CASE("testCopyConstructor");

   SystemInfo original;
   SystemInfo copy(original);

   requireStringEquals(original.sysName(), copy.sysName(), "copy should have the same sysName");
   requireStringEquals(original.nodeName(), copy.nodeName(), "copy should have the same nodeName");
   requireStringEquals(original.release(), copy.release(), "copy should have the same release");
   requireStringEquals(original.version(), copy.version(), "copy should have the same version");
   requireStringEquals(original.machine(), copy.machine(), "copy should have the same machine");
   require(original.retrievedSystemInfo() == copy.retrievedSystemInfo(), "copy should have the same retrievedSystemInfo status");
}

//******************************************************************************

void TestSystemInfo::testAssignmentOperator() {
   TEST_CASE("testAssignmentOperator");

   SystemInfo source;
   SystemInfo target;
   target = source;

   requireStringEquals(source.sysName(), target.sysName(), "target should have the same sysName after assignment");
   requireStringEquals(source.machine(), target.machine(), "target should have the same machine after assignment");

   // self-assignment should be a safe no-op
   target = target;
   requireStringEquals(source.sysName(), target.sysName(), "self-assignment should not corrupt state");
}

//******************************************************************************

void TestSystemInfo::testSysName() {
   TEST_CASE("testSysName");

   SystemInfo si;
   requireNonEmptyString(si.sysName(), "sysName should be non-empty on a successfully retrieved SystemInfo");
}

//******************************************************************************

void TestSystemInfo::testNodeName() {
   TEST_CASE("testNodeName");

   SystemInfo si;
   requireNonEmptyString(si.nodeName(), "nodeName should be non-empty on a successfully retrieved SystemInfo");
}

//******************************************************************************

void TestSystemInfo::testRelease() {
   TEST_CASE("testRelease");

   SystemInfo si;
   requireNonEmptyString(si.release(), "release should be non-empty on a successfully retrieved SystemInfo");
}

//******************************************************************************

void TestSystemInfo::testVersion() {
   TEST_CASE("testVersion");

   SystemInfo si;
   requireNonEmptyString(si.version(), "version should be non-empty on a successfully retrieved SystemInfo");
}

//******************************************************************************

void TestSystemInfo::testMachine() {
   TEST_CASE("testMachine");

   SystemInfo si;
   requireNonEmptyString(si.machine(), "machine should be non-empty on a successfully retrieved SystemInfo");
}

//******************************************************************************

void TestSystemInfo::testRetrievedSystemInfo() {
   TEST_CASE("testRetrievedSystemInfo");

   SystemInfo si;
   require(si.retrievedSystemInfo(), "retrievedSystemInfo should be true after a successful construction");
}

//******************************************************************************
