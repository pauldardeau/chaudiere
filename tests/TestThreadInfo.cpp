// Copyright Paul Dardeau, SwampBits LLC 2015
// BSD License

#include "TestThreadInfo.h"
#include "ThreadInfo.h"

using namespace chaudiere;

//******************************************************************************

TestThreadInfo::TestThreadInfo() :
   poivre::TestSuite("TestThreadInfo") {
}

//******************************************************************************

void TestThreadInfo::runTests() {
   testConstructor();
   testCopyConstructor();
   testAssignmentOperator();
}

//******************************************************************************

void TestThreadInfo::testConstructor() {
   TEST_CASE("testConstructor");

   ThreadInfo ti("thread-1");
   requireStringEquals("thread-1", ti.threadId, "threadId should match the constructor argument");
   requireFalse(ti.isBusy, "isBusy should default to false");
}

//******************************************************************************

void TestThreadInfo::testCopyConstructor() {
   TEST_CASE("testCopyConstructor");

   ThreadInfo original("thread-2");
   original.isBusy = true;

   ThreadInfo copy(original);
   requireStringEquals("thread-2", copy.threadId, "copy should have the same threadId");
   require(copy.isBusy, "copy should have the same isBusy value");
}

//******************************************************************************

void TestThreadInfo::testAssignmentOperator() {
   TEST_CASE("testAssignmentOperator");

   ThreadInfo source("thread-3");
   source.isBusy = true;

   ThreadInfo target("thread-unassigned");
   target = source;

   requireStringEquals("thread-3", target.threadId, "target should have the same threadId after assignment");
   require(target.isBusy, "target should have the same isBusy value after assignment");

   // self-assignment should be a safe no-op
   target = target;
   requireStringEquals("thread-3", target.threadId, "self-assignment should not corrupt state");
}

//******************************************************************************
