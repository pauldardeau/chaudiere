// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTSTDTHREADINGFACTORY_H
#define CHAUDIERE_TESTSTDTHREADINGFACTORY_H

#include "TestSuite.h"

namespace chaudiere
{

class TestStdThreadingFactory : public poivre::TestSuite
{
protected:
   void runTests();

   void testCreateMutex();
   void testCreateThreadWithName();
   void testCreateThreadWithRunnable();
   void testCreateConditionVariable();
   void testCreateThreadPoolDispatcher();

public:
   TestStdThreadingFactory();

};

}

#endif
