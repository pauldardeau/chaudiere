// Copyright Paul Dardeau, SwampBits LLC 2015
// BSD License

#ifndef CHAUDIERE_TESTAUTOPOINTER_H
#define CHAUDIERE_TESTAUTOPOINTER_H

#include "TestSuite.h"

namespace chaudiere
{

class TestAutoPointer : public TestSuite
{
protected:
   void runTests();

   void testConstructor();
   void testOperatorArrow();
   void testAssign();
   void testHaveObject();
   void testDestructor();

public:
   TestAutoPointer();


};

}

#endif
