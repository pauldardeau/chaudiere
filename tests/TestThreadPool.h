// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef TESTTHREADPOOL_H
#define TESTTHREADPOOL_H

#include "TestSuite.h"


class TestThreadPool : public TestSuite
{
protected:
   void runTests();
   
   void testConstructor();
   void testStart();
   void testStop();
   void testAddRequest();
   void testGetNumberWorkers();
   void testAddWorkers();
   void testRemoveWorkers();

public:
   TestThreadPool();
   

};

#endif
