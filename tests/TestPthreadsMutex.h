// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef TESTPTHREADSMUTEX_H
#define TESTPTHREADSMUTEX_H

#include "TestSuite.h"


class TestPthreadsMutex : public TestSuite
{
protected:
   void runTests();
   
   void testConstructor();
   void testConstructorWithName();
   
   void testLock();
   void testUnlock();
   void testHaveValidMutex();
   void testGetPlatformPrimitive();
   void testGetName();
   void testIsLocked();

public:
   TestPthreadsMutex();
   
};


#endif
