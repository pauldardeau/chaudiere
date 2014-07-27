// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef TESTTHREADPOOLWORKER_H
#define TESTTHREADPOOLWORKER_H

#include "TestSuite.h"


class TestThreadPoolWorker : public TestSuite
{
protected:
   void runTests();
   
   void testConstructor();
   void testRun();

public:
   TestThreadPoolWorker();
   

};

#endif
