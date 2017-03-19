// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTTHREADPOOL_H
#define CHAUDIERE_TESTTHREADPOOL_H

#include "TestSuite.h"

namespace chaudiere
{

class TestThreadPool : public poivre::TestSuite
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

}

#endif
