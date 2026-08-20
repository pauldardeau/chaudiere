// Copyright Paul Dardeau, SwampBits LLC 2015
// BSD License

#ifndef CHAUDIERE_TESTCHARBUFFER_H
#define CHAUDIERE_TESTCHARBUFFER_H

#include "TestSuite.h"

namespace chaudiere
{

class TestCharBuffer : public poivre::TestSuite
{
protected:
   void runTests();

   void testDefaultConstructor();
   void testConstructorWithSize();
   void testNullAt();
   void testAllocateBuffer();
   void testEnsureCapacityWhenEmpty();
   void testEnsureCapacityGrowing();
   void testEnsureCapacityAlreadySufficient();
   void testData();
   void testSize();

public:
   TestCharBuffer();

};

}

#endif
