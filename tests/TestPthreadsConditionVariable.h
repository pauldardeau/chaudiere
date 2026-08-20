// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTPTHREADSCONDITIONVARIABLE_H
#define CHAUDIERE_TESTPTHREADSCONDITIONVARIABLE_H

#include "TestSuite.h"

namespace chaudiere
{

class TestPthreadsConditionVariable : public poivre::TestSuite
{
protected:
   void runTests();

   void testConstructor();
   void testConstructorWithName();
   void testGetName();
   void testWaitWithNullMutex();
   void testWaitWithWrongMutexType();
   void testNotifyOneNoWaiters();
   void testWait();
   void testNotifyAll();

public:
   TestPthreadsConditionVariable();

};

}

#endif
