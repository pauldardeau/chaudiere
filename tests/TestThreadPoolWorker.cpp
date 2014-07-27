// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestThreadPoolWorker.h"
#include "ThreadPoolWorker.h"

//******************************************************************************

TestThreadPoolWorker::TestThreadPoolWorker() :
   TestSuite("TestThreadPoolWorker")
{
}

//******************************************************************************

void TestThreadPoolWorker::runTests()
{
   testConstructor();
   testRun();
}

//******************************************************************************

void TestThreadPoolWorker::testConstructor()
{
   TEST_CASE("testConstructor");
   
}

//******************************************************************************

void TestThreadPoolWorker::testRun()
{
   TEST_CASE("testRun");
   
}

//******************************************************************************

