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

   // parsing
   void testParseInt();
   void testParseLong();
   void testParseFloat();
   void testParseDouble();

   // toString
   void testToStringWithInt();
   void testToStringWithLong();
   void testToStringWithULong();

   // strip
   void testStrip();
   void testStripWithChar();

   void testStartsWith();
   void testEndsWith();
   void testContainsString();

   // upper/lower case
   void testToUpperCase();
   void testToLowerCase();

   void testReplaceAll();

   // strip
   void testStripInPlace();
   void testStripTrailing();
   void testStripLeading();
   void testTrimLeadingSpaces();
   void testTrim();

   // padding
   void testPadRight();
   void testPadLeft();

   // makeStringOfChar
   void testMakeStringOfChar();

   // split
   void testSplit();

public:
   TestStrUtils();

};

}

#endif

