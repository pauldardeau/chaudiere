// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTTHREADINGFACTORY_H
#define CHAUDIERE_TESTTHREADINGFACTORY_H

#include "TestSuite.h"

namespace chaudiere
{

class TestThreadingFactory : public poivre::TestSuite
{
protected:
   void runTests();

   void testGetThreadingFactory();
   void testSetThreadingFactory();

public:
   TestThreadingFactory();

};

}

#endif
