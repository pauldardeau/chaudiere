// Copyright Paul Dardeau, SwampBits LLC 2015
// BSD License

#ifndef CHAUDIERE_OPTIONPARSER_H
#define CHAUDIERE_OPTIONPARSER_H

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
   void testAddBooleanOption();
   void testAddOption();
   void testHasOption();
   void testGetOptionValue();
   void testHasBooleanOption();
   void testParseArgs();
   
public:
   TestOptionParser();

};

}

#endif
