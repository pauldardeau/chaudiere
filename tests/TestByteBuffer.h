// Copyright Paul Dardeau, SwampBits LLC 2015
// BSD License

#ifndef CHAUDIERE_TESTBYTEBUFFER_H
#define CHAUDIERE_TESTBYTEBUFFER_H

#include "TestSuite.h"


namespace chaudiere
{

class TestByteBuffer : public TestSuite
{
protected:
   void runTests();
   

public:
   TestByteBuffer();
   
   void testDefaultConstructor();
   void testSizeConstructor();
   void testCopyConstructor();
   void testAssignmentOperator();
   void testTake();
   void testRelease();
   void testData();
   void testSize();
   void testClear();

};

}

#endif
