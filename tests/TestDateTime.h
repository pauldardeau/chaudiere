// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTDATETIME_H
#define CHAUDIERE_TESTDATETIME_H

#include "TestSuite.h"

namespace chaudiere
{

class TestDateTime : public poivre::TestSuite
{
protected:
   void runTests();

   void testDefaultConstructor();
   void testConstructorWithDummy();
   void testConstructorWithComponents();
   void testConstructorWithUnixTime();
   void testConstructorWithString();
   void testCopyConstructor();
   void testAssignmentOperator();
   void testEqualityOperator();
   void testLessThanOperator();
   void testTimeIntervalSinceDate();
   void testTimeIntervalSince1970();
   void testFormattedString();
   void testUnformattedString();
   void testYearAccessors();
   void testMonthAccessors();
   void testDayAccessors();
   void testHourAccessors();
   void testMinuteAccessors();
   void testSecondAccessors();
   void testMicrosecondAccessors();
   void testWeekDayAccessors();
   void testDateFromString();
   void testUnixTimeValue();
   void testPopulateFromUnixTime();
   void testGmtDateTime();

public:
   TestDateTime();

};

}

#endif
