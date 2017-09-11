// Copyright Paul Dardeau, SwampBits LLC 2015
// BSD License

#ifndef CHAUDIERE_TESTOPTIONPARSER_H
#define CHAUDIERE_TESTOPTIONPARSER_H

#include "TestSuite.h"

namespace chaudiere
{

class TestOptionParser : public poivre::TestSuite
{
protected:
   void runTests();
   
   void testConstructor();
   void testCopyConstructor();
   void testAssignmentOperator();
   void testAddFlag();
   void testAddOption();
   void testHasOption();
   void testGetOptionValue();
   void testHasFlag();
   void testParseArgs();
   
public:
   TestOptionParser();

};

}

#endif
