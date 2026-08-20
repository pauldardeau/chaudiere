// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTSTDTHREAD_H
#define CHAUDIERE_TESTSTDTHREAD_H

#include "TestSuite.h"

namespace chaudiere
{

class TestStdThread : public poivre::TestSuite
{
protected:
   void runTests();

   void testDefaultConstructor();
   void testConstructorWithName();
   void testConstructorWithRunnable();
   void testConstructorWithRunnableAndName();
   void testGetName();
   void testStartAndJoin();
   void testIsAliveDuringRun();
   void testGetHandle();
   void testGetStdThreadId();
   void testJoinWithoutStart();

public:
   TestStdThread();

};

}

#endif
