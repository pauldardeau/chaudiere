// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestThreadPoolWorker.h"
#include "ThreadPoolWorker.h"

using namespace chaudiere;

//******************************************************************************

TestThreadPoolWorker::TestThreadPoolWorker() :
   TestSuite("TestThreadPoolWorker") {
}

//******************************************************************************

void TestThreadPoolWorker::runTests() {
   testConstructor();
   testRun();
}

//******************************************************************************

void TestThreadPoolWorker::testConstructor() {
   TEST_CASE("testConstructor");
   //TODO: implement testConstructor
}

//******************************************************************************

void TestThreadPoolWorker::testRun() {
   TEST_CASE("testRun");
   //TODO: implement testRun
}

//******************************************************************************

