// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTPTHREADSTHREADINGFACTORY_H
#define CHAUDIERE_TESTPTHREADSTHREADINGFACTORY_H

#include "TestSuite.h"

namespace chaudiere
{

class TestPthreadsThreadingFactory : public poivre::TestSuite
{
protected:
   void runTests();

   void testCreateMutex();
   void testCreateThreadWithName();
   void testCreateThreadWithRunnable();
   void testCreateConditionVariable();
   void testCreateThreadPoolDispatcher();

public:
   TestPthreadsThreadingFactory();

};

}

#endif
