// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef TESTTHREADPOOLQUEUE_H
#define TESTTHREADPOOLQUEUE_H

#include "TestSuite.h"


class TestThreadPoolQueue : public TestSuite
{
protected:
   void runTests();
   
   void testConstructor();
   void testAddRequest();
   void testTakeRequest();
   void testShutDown();
   void testIsRunning();
   void testIsEmpty();

public:
   TestThreadPoolQueue();
   

};

#endif
