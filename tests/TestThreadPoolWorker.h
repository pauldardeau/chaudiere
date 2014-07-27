// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTTHREADPOOLWORKER_H
#define CHAUDIERE_TESTTHREADPOOLWORKER_H

#include "TestSuite.h"

namespace chaudiere
{

class TestThreadPoolWorker : public TestSuite
{
protected:
   void runTests();
   
   void testConstructor();
   void testRun();

public:
   TestThreadPoolWorker();   

};

}

#endif
