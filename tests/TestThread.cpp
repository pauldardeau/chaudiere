// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestThread.h"
#include "Thread.h"

//******************************************************************************

TestThread::TestThread() :
   TestSuite("TestThread")
{
}

//******************************************************************************

void TestThread::runTests()
{
   testConstructor();
   testConstructorWithRunnable();
   testSetPoolWorkerStatus();
   testIsPoolWorker();
   testStart();
   testRun();
   testIsAlive();
   testGetExitCode();
   testRegisterThreadCompletionObserver();
   testClearThreadCompletionObserver();
   testGetRunnable();
   testGetHandle();
   testSetAttribute();
   testHasAttribute();
   testGetAttribute();
   testClearAttribute();
}

//******************************************************************************

void TestThread::testConstructor()
{
   TEST_CASE("testConstructor");
   
}

//******************************************************************************

void TestThread::testConstructorWithRunnable()
{
   TEST_CASE("testConstructorWithRunnable");
}

//******************************************************************************

void TestThread::testSetPoolWorkerStatus()
{
   TEST_CASE("testSetPoolWorkerStatus");
   
}

//******************************************************************************

void TestThread::testIsPoolWorker()
{
   TEST_CASE("testIsPoolWorker");
   
}

//******************************************************************************

void TestThread::testStart()
{
   TEST_CASE("testStart");
   
}

//******************************************************************************

void TestThread::testRun()
{
   TEST_CASE("testRun");
   
}

//******************************************************************************

void TestThread::testIsAlive()
{
   TEST_CASE("testIsAlive");
   
}

//******************************************************************************

void TestThread::testGetExitCode()
{
   TEST_CASE("testGetExitCode");
   
}

//******************************************************************************

void TestThread::testRegisterThreadCompletionObserver()
{
   TEST_CASE("testRegisterThreadCompletionObserver");
   
}

//******************************************************************************

void TestThread::testClearThreadCompletionObserver()
{
   TEST_CASE("testClearThreadCompletionObserver");
   
}

//******************************************************************************

void TestThread::testGetRunnable()
{
   TEST_CASE("testGetRunnable");
   
}

//******************************************************************************

void TestThread::testGetHandle()
{
   TEST_CASE("testGetHandle");
   
}

//******************************************************************************

void TestThread::testSetAttribute()
{
   TEST_CASE("testSetAttribute");
   
}

//******************************************************************************

void TestThread::testHasAttribute()
{
   TEST_CASE("testHasAttribute");
   
}

//******************************************************************************

void TestThread::testGetAttribute()
{
   TEST_CASE("testGetAttribute");
   
}

//******************************************************************************

void TestThread::testClearAttribute()
{
   TEST_CASE("testClearAttribute");
   
}

//******************************************************************************

