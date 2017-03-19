// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTTHREADPOOLQUEUE_H
#define CHAUDIERE_TESTTHREADPOOLQUEUE_H

#include "TestSuite.h"

namespace chaudiere
{

class TestThreadPoolQueue : public poivre::TestSuite
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

}

#endif
