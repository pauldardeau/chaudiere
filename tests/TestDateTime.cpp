// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <cmath>
#include <ctime>

#include "TestDateTime.h"
#include "DateTime.h"

using namespace chaudiere;

//******************************************************************************

TestDateTime::TestDateTime() :
   poivre::TestSuite("TestDateTime") {
}

//******************************************************************************

void TestDateTime::runTests() {
   testDefaultConstructor();
   testConstructorWithDummy();
   testConstructorWithComponents();
   testConstructorWithUnixTime();
   testConstructorWithString();
   testCopyConstructor();
   testAssignmentOperator();
   testEqualityOperator();
   testLessThanOperator();
   testTimeIntervalSinceDate();
   testTimeIntervalSince1970();
   testFormattedString();
   testUnformattedString();
   testYearAccessors();
   testMonthAccessors();
   testDayAccessors();
   testHourAccessors();
   testMinuteAccessors();
   testSecondAccessors();
   testMicrosecondAccessors();
   testWeekDayAccessors();
   testDateFromString();
   testUnixTimeValue();
   testPopulateFromUnixTime();
   testGmtDateTime();
}

//******************************************************************************

void TestDateTime::testDefaultConstructor() {
   TEST_CASE("testDefaultConstructor");

   DateTime now;
   const double nowUnixTime = now.timeIntervalSince1970();
   const double realUnixTime = (double) ::time(nullptr);

   require(std::fabs(nowUnixTime - realUnixTime) < 5.0, "default-constructed DateTime should reflect the current time");
}

//******************************************************************************

void TestDateTime::testConstructorWithDummy() {
   TEST_CASE("testConstructorWithDummy");

   DateTime dt(0);
   require(0 == dt.getYear(), "dummy constructor should leave year at 0");
   require(0 == dt.getMonth(), "dummy constructor should leave month at 0");
   require(0 == dt.getDay(), "dummy constructor should leave day at 0");
}

//******************************************************************************

void TestDateTime::testConstructorWithComponents() {
   TEST_CASE("testConstructorWithComponents");

   DateTime dt(2023, 6, 15, 9, 5, 3, 123456);
   require(2023 == dt.getYear(), "year should match constructor argument");
   require(6 == dt.getMonth(), "month should match constructor argument");
   require(15 == dt.getDay(), "day should match constructor argument");
   require(9 == dt.getHour(), "hour should match constructor argument");
   require(5 == dt.getMinute(), "minute should match constructor argument");
   require(3 == dt.getSecond(), "second should match constructor argument");
   require(123456 == dt.getMicrosecond(), "microsecond should match constructor argument");
}

//******************************************************************************

void TestDateTime::testConstructorWithUnixTime() {
   TEST_CASE("testConstructorWithUnixTime");

   DateTime dt(1700000000.0);
   require(dt.getYear() >= 2023, "year derived from a 2023-era unix time should be plausible");
   require(1700000000.0 == dt.timeIntervalSince1970(), "timeIntervalSince1970 should return exactly the constructor argument");
}

//******************************************************************************

void TestDateTime::testConstructorWithString() {
   TEST_CASE("testConstructorWithString");

   // unformatted date/time string: yyyymmddhhmmss
   DateTime dt(std::string("20230615090503"));
   require(2023 == dt.getYear(), "year should be parsed from the unformatted string");
   require(6 == dt.getMonth(), "month should be parsed from the unformatted string");
   require(15 == dt.getDay(), "day should be parsed from the unformatted string");
   require(9 == dt.getHour(), "hour should be parsed from the unformatted string");
   require(5 == dt.getMinute(), "minute should be parsed from the unformatted string");
   require(3 == dt.getSecond(), "second should be parsed from the unformatted string");
}

//******************************************************************************

void TestDateTime::testCopyConstructor() {
   TEST_CASE("testCopyConstructor");

   DateTime original(2023, 6, 15, 9, 5, 3, 123456);
   DateTime copy(original);

   require(original.getYear() == copy.getYear(), "copy should have the same year");
   require(original.getMonth() == copy.getMonth(), "copy should have the same month");
   require(original.getDay() == copy.getDay(), "copy should have the same day");
   require(original.getHour() == copy.getHour(), "copy should have the same hour");
   require(original.getMinute() == copy.getMinute(), "copy should have the same minute");
   require(original.getSecond() == copy.getSecond(), "copy should have the same second");
   require(original.getMicrosecond() == copy.getMicrosecond(), "copy should have the same microsecond");
}

//******************************************************************************

void TestDateTime::testAssignmentOperator() {
   TEST_CASE("testAssignmentOperator");

   DateTime source(2023, 6, 15, 9, 5, 3, 123456);
   DateTime target(0);
   target = source;

   require(2023 == target.getYear(), "target should have the same year after assignment");
   require(6 == target.getMonth(), "target should have the same month after assignment");

   // self-assignment should be a safe no-op
   target = target;
   require(2023 == target.getYear(), "self-assignment should not corrupt state");
}

//******************************************************************************

void TestDateTime::testEqualityOperator() {
   TEST_CASE("testEqualityOperator");

   DateTime a(2023, 6, 15, 9, 5, 3, 123456);
   DateTime b(2023, 6, 15, 9, 5, 3, 123456);
   DateTime c(2023, 6, 15, 9, 5, 4, 123456);

   require(a == b, "two DateTimes with identical components should be equal");
   requireFalse(a == c, "DateTimes differing by even one component should not be equal");
}

//******************************************************************************

void TestDateTime::testLessThanOperator() {
   TEST_CASE("testLessThanOperator");

   DateTime earlier(2023, 6, 15, 9, 5, 3, 0);
   DateTime later(2023, 6, 15, 9, 5, 4, 0);

   require(earlier < later, "an earlier DateTime should be less than a later one");
   requireFalse(later < earlier, "a later DateTime should not be less than an earlier one");
}

//******************************************************************************

void TestDateTime::testTimeIntervalSinceDate() {
   TEST_CASE("testTimeIntervalSinceDate");

   DateTime a(1700000100.0);
   DateTime b(1700000000.0);

   require(100.0 == a.timeIntervalSinceDate(b), "timeIntervalSinceDate should return the difference in unix time");
}

//******************************************************************************

void TestDateTime::testTimeIntervalSince1970() {
   TEST_CASE("testTimeIntervalSince1970");

   DateTime dt(1700000000.0);
   require(1700000000.0 == dt.timeIntervalSince1970(), "timeIntervalSince1970 should return the underlying unix time");
}

//******************************************************************************

void TestDateTime::testFormattedString() {
   TEST_CASE("testFormattedString");

   DateTime dt(2023, 6, 15, 9, 5, 3, 123456);
   requireStringEquals("2023-06-15 09:05:03.123456", dt.formattedString(), "formattedString should format component-based dates correctly");

   // Known gap (documented in DateTime.cpp as "TODO: what if we only have
   // unix time populated?"): when a DateTime has its unix-time value
   // populated (the default constructor's fast path, or DateTime(double)),
   // formattedString() returns an empty string instead of deriving the
   // calendar fields first -- even though DateTime(double) *does* also
   // populate the calendar fields via populateFromUnixTime().
   DateTime unixOnly(1700000000.0);
   require(unixOnly.formattedString().empty(), "formattedString currently returns empty for a DateTime with a unix-time value populated (documented gap)");
}

//******************************************************************************

void TestDateTime::testUnformattedString() {
   TEST_CASE("testUnformattedString");

   DateTime dt(2023, 6, 15, 9, 5, 3, 123456);
   requireStringEquals("20230615090503123456", dt.unformattedString(), "unformattedString should format component-based dates correctly");
}

//******************************************************************************

void TestDateTime::testYearAccessors() {
   TEST_CASE("testYearAccessors");

   DateTime dt(0);
   dt.setYear(2024);
   require(2024 == dt.getYear(), "getYear should reflect setYear");
}

//******************************************************************************

void TestDateTime::testMonthAccessors() {
   TEST_CASE("testMonthAccessors");

   DateTime dt(0);
   dt.setMonth(11);
   require(11 == dt.getMonth(), "getMonth should reflect setMonth");
}

//******************************************************************************

void TestDateTime::testDayAccessors() {
   TEST_CASE("testDayAccessors");

   DateTime dt(0);
   dt.setDay(22);
   require(22 == dt.getDay(), "getDay should reflect setDay");
}

//******************************************************************************

void TestDateTime::testHourAccessors() {
   TEST_CASE("testHourAccessors");

   DateTime dt(0);
   dt.setHour(17);
   require(17 == dt.getHour(), "getHour should reflect setHour");
}

//******************************************************************************

void TestDateTime::testMinuteAccessors() {
   TEST_CASE("testMinuteAccessors");

   DateTime dt(0);
   dt.setMinute(42);
   require(42 == dt.getMinute(), "getMinute should reflect setMinute");
}

//******************************************************************************

void TestDateTime::testSecondAccessors() {
   TEST_CASE("testSecondAccessors");

   DateTime dt(0);
   dt.setSecond(59);
   require(59 == dt.getSecond(), "getSecond should reflect setSecond");
}

//******************************************************************************

void TestDateTime::testMicrosecondAccessors() {
   TEST_CASE("testMicrosecondAccessors");

   DateTime dt(0);
   dt.setMicrosecond(999999);
   require(999999 == dt.getMicrosecond(), "getMicrosecond should reflect setMicrosecond");
}

//******************************************************************************

void TestDateTime::testWeekDayAccessors() {
   TEST_CASE("testWeekDayAccessors");

   DateTime dt(0);
   dt.setWeekDay(3);
   require(3 == dt.getWeekDay(), "getWeekDay should reflect setWeekDay");
}

//******************************************************************************

void TestDateTime::testDateFromString() {
   TEST_CASE("testDateFromString");

   DateTime dt(0);
   DateTime::dateFromString(&dt, "2023-06-15 12:30:45");

   require(2023 == dt.getYear(), "dateFromString should parse the year");
   require(6 == dt.getMonth(), "dateFromString should parse the month");
   require(15 == dt.getDay(), "dateFromString should parse the day");
   require(12 == dt.getHour(), "dateFromString should parse the hour");
   require(30 == dt.getMinute(), "dateFromString should parse the minute");
   require(45 == dt.getSecond(), "dateFromString should parse the second");
}

//******************************************************************************

void TestDateTime::testUnixTimeValue() {
   TEST_CASE("testUnixTimeValue");

   // fast path: unix time already populated
   DateTime unixOnly(1700000000.0);
   require(1700000000.0 == DateTime::unixTimeValue(unixOnly), "unixTimeValue should return the already-populated unix time directly");

   // computed path: only calendar components are known -- verify a
   // round trip through the unix time and back reproduces the same
   // components (both directions use local time, so this is
   // timezone-independent)
   DateTime componentsDate(2023, 6, 15, 12, 30, 45, 0);
   const double computedUnixTime = DateTime::unixTimeValue(componentsDate);
   require(computedUnixTime > 0, "computed unix time should be positive for a date after 1970");

   DateTime roundTripped(computedUnixTime);
   require(2023 == roundTripped.getYear(), "round-tripped year should match");
   require(6 == roundTripped.getMonth(), "round-tripped month should match");
   require(15 == roundTripped.getDay(), "round-tripped day should match");
   require(12 == roundTripped.getHour(), "round-tripped hour should match");
   require(30 == roundTripped.getMinute(), "round-tripped minute should match");
   require(45 == roundTripped.getSecond(), "round-tripped second should match");
}

//******************************************************************************

void TestDateTime::testPopulateFromUnixTime() {
   TEST_CASE("testPopulateFromUnixTime");

   DateTime dt(0);
   require(DateTime::populateFromUnixTime(dt, 1700000000.0), "populateFromUnixTime should succeed for a valid unix time");
   require(dt.getYear() >= 2023, "populated year should be plausible for the given unix time");
}

//******************************************************************************

void TestDateTime::testGmtDateTime() {
   TEST_CASE("testGmtDateTime");

   DateTime* dt = DateTime::gmtDateTime();
   require(nullptr != dt, "gmtDateTime should return a non-null instance");
   require(dt->getYear() >= 2024, "GMT year should be plausible for the current date");
   delete dt;
}

//******************************************************************************
