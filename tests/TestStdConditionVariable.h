// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTSTDCONDITIONVARIABLE_H
#define CHAUDIERE_TESTSTDCONDITIONVARIABLE_H

#include "TestSuite.h"

namespace chaudiere
{

class TestStdConditionVariable : public poivre::TestSuite
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
   TestStdConditionVariable();

};

}

#endif
