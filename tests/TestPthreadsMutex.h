// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTPTHREADSMUTEX_H
#define CHAUDIERE_TESTPTHREADSMUTEX_H

#include "TestSuite.h"

namespace chaudiere
{

class TestPthreadsMutex : public poivre::TestSuite
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

}

#endif
