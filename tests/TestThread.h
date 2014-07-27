// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef TESTTHREAD_H
#define TESTTHREAD_H

#include "TestSuite.h"


class TestThread : public TestSuite
{
protected:
   void runTests();
   
   void testConstructor();
   void testConstructorWithRunnable();
   
   void testSetPoolWorkerStatus();
   void testIsPoolWorker();
   
   void testStart();
   void testRun();
   
   void testIsAlive();
   
   void testGetExitCode();
   
   void testRegisterThreadCompletionObserver();
   void testClearThreadCompletionObserver();
   
   void testGetRunnable();
   
   void testGetHandle();
   
   void testSetAttribute();
   void testHasAttribute();
   void testGetAttribute();
   void testClearAttribute();

public:
   TestThread();
   

};

#endif
