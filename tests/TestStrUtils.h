// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTSTRUTILS_H
#define CHAUDIERE_TESTSTRUTILS_H

#include "TestSuite.h"

namespace chaudiere
{

class TestStrUtils : public TestSuite
{
protected:
   void runTests();
   
   void testStrip();
   void testStripWithChar();
   
   void testStartsWith();
   void testEndsWith();
   void testContainsString();
   
   void testToUpperCase();
   void testToLowerCase();
   
   void testReplaceAll();
   
   void testStripInPlace();
   void testStripTrailing();
   void testStripLeading();
   void testTrimLeadingSpaces();
   
   void testPadRight();

public:
   TestStrUtils();
   
};

}

#endif
