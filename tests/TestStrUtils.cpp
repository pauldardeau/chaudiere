// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <string.h>
#include <cmath>
#include <cstdio>
#include <clocale>
#include <limits>
#include <string>
#include <string_view>
#include <locale.h>
#ifdef __APPLE__
#include <xlocale.h>
#endif

#include "TestStrUtils.h"
#include "TestMacros.h"
#include "StrUtils.h"
#include "NumberFormatException.h"

static const std::string emptyString = "";

using namespace std;
using namespace chaudiere;

//******************************************************************************

TestStrUtils::TestStrUtils() :
   poivre::TestSuite("TestStrUtils") {
}

//******************************************************************************

void TestStrUtils::runTests() {
   // parsing
   testParseInt();
   testParseLong();
   testParseLongRange();
   testLanguageLevel();
   testParseFloat();
   testParseDouble();

   // toString
   testToStringWithInt();
   testToStringWithLong();
   testToStringWithULong();
   testToStringWithLongLong();

   // strip
   testStrip();
   testStripWithChar();

   testStartsWith();
   testEndsWith();
   testStartsAndEndsWith();
   testContainsString();

   // upper/lower case
   testToUpperCase();
   testToLowerCase();

   // search & replace
   testReplaceAll();

   // strip
   testStripInPlace();
   testStripTrailing();
   testStripLeading();
   testTrimLeadingSpaces();
   testTrim();

   // padding
   testPadRight();
   testPadLeft();

   // makeStringOfChar
   testMakeStringOfChar();

   // split
   testSplit();
}

//******************************************************************************

class ParseRunner : public poivre::Runnable {
public:
   explicit ParseRunner(const std::string& arg) :
      m_arg(arg) {
   }

   ParseRunner(const ParseRunner& copy) :
      m_arg(copy.m_arg) {
   }

   ~ParseRunner() {}

   ParseRunner& operator=(const ParseRunner& copy) {
      if (this == &copy) {
         return *this;
      }

      m_arg = copy.m_arg;
      return *this;
   }

   virtual void run() = 0;

protected:
   std::string m_arg;
};

class ParseIntRunner : public ParseRunner {
public:
   ParseIntRunner(const std::string& arg) :
      ParseRunner(arg) {
   }

   virtual void run() {
      StrUtils::parseInt(m_arg);
   }
};

class ParseLongRunner : public ParseRunner {
public:
   ParseLongRunner(const std::string& arg) :
      ParseRunner(arg) {
   }

   virtual void run() {
      StrUtils::parseLong(m_arg);
   }
};

class ParseFloatRunner : public ParseRunner {
public:
   ParseFloatRunner(const std::string& arg) :
      ParseRunner(arg) {
   }

   virtual void run() {
      StrUtils::parseFloat(m_arg);
   }
};

class ParseDoubleRunner : public ParseRunner {
public:
   ParseDoubleRunner(const std::string& arg) :
      ParseRunner(arg) {
   }

   virtual void run() {
      StrUtils::parseDouble(m_arg);
   }
};

//*****************************************************************************
//*****************************************************************************

void TestStrUtils::testParseInt() {
   TEST_CASE("parseInt");

   require(3 == StrUtils::parseInt(string("3")), "1 digit positive integer");
   require(-5 == StrUtils::parseInt(string("-5")), "1 digit negative integer");
   require(0 == StrUtils::parseInt(string("0")), "zero");
   require(35 == StrUtils::parseInt(string("35")), "2 digit positive integer");
   require(121 == StrUtils::parseInt(string("121")), "3 digit positive integer");
   require(4096 == StrUtils::parseInt(string("4096")), "4 digit positive integer");
   require(65535 == StrUtils::parseInt(string("65535")), "5 digit positive integer");

   // try largest (absolute value) signed value for 32 bits
   require(2147483647 == StrUtils::parseInt(string("2147483647")),
           "large positive int");
   require(-2147483648UL == StrUtils::parseInt(string("-2147483648")),
           "negative int with large absolute value");

   requireException("NumberFormatException", new ParseIntRunner(""),
                    "empty string");
   requireException("NumberFormatException", new ParseIntRunner("x"), "letter");
   requireException("NumberFormatException", new ParseIntRunner("y123"),
                    "leading char");
   requireException("NumberFormatException", new ParseIntRunner("456t"),
                    "trailing char");
}

//******************************************************************************

void TestStrUtils::testParseLong() {
   TEST_CASE("parseLong");

   require(3L == StrUtils::parseLong(string("3")), "1 digit positive long");
   require(-5L == StrUtils::parseLong(string("-5")), "1 digit negative long");
   require(0L == StrUtils::parseLong(string("0")), "zero");
   require(35L == StrUtils::parseLong(string("35")), "2 digit positive long");
   require(121L == StrUtils::parseLong(string("121")), "3 digit positive long");
   require(4096L == StrUtils::parseLong(string("4096")), "4 digit positive long");
   require(65535L == StrUtils::parseLong(string("65535")), "5 digit positive long");

   // try largest (absolute value) signed value for 32 bits
   require(2147483647L == StrUtils::parseLong(string("2147483647")),
           "large positive long");
   require(-2147483648UL == StrUtils::parseLong(string("-2147483648")),
           "negative long with large absolute value");

   requireException("NumberFormatException", new ParseLongRunner(""),
                    "empty string");
   requireException("NumberFormatException", new ParseLongRunner("x"), "letter");
   requireException("NumberFormatException", new ParseLongRunner("y123"),
                    "leading char");
   requireException("NumberFormatException", new ParseLongRunner("456t"),
                    "trailing char");
}

//******************************************************************************

void TestStrUtils::testParseLongRange() {
   TEST_CASE("parseLongRange");

   // parseLong() returns a long, which is 32 bits on Windows (LLP64) and 64
   // bits on Linux/macOS. Out of range is always an error, never a clamped
   // value: the pre-C++11 atol() path this must not take would have returned
   // LONG_MAX (2147483647) here on Windows.
   if constexpr (sizeof(long) == 4) {
      requireException("NumberFormatException", new ParseLongRunner("4294967296"),
                       "2^32 does not fit in a 32-bit long");
      requireException("NumberFormatException", new ParseLongRunner("2147483648"),
                       "one past the largest 32-bit long");
      requireException("NumberFormatException", new ParseLongRunner("-2147483649"),
                       "one below the smallest 32-bit long");
   } else {
      require(4294967296L == StrUtils::parseLong(string("4294967296")),
              "2^32 fits in a 64-bit long");
      require(2147483648L == StrUtils::parseLong(string("2147483648")),
              "one past the largest 32-bit long fits in a 64-bit long");
   }

   requireException("NumberFormatException",
                    new ParseLongRunner("99999999999999999999999"),
                    "far beyond any long");
}

//******************************************************************************

void TestStrUtils::testLanguageLevel() {
   TEST_CASE("languageLevel");

   // StrUtils.h/.cpp choose their code from __cplusplus, and MSVC reports
   // 199711L whatever /std: says unless /Zc:__cplusplus is passed (the
   // chaudiere target adds it for everything that links it)
   require(__cplusplus >= 202002L,
           "__cplusplus should report C++20 or later");

   // the string_view overloads are declared only when __cplusplus >= 202002L,
   // so this call compiling and working shows that the header, as seen by
   // this test, took the C++20 branch
   require(7 == StrUtils::parseInt(std::string_view("7")),
           "string_view overload of parseInt");
   require(StrUtils::startsWith(std::string_view("abc"), std::string_view("ab")),
           "string_view overload of startsWith");
}

//******************************************************************************

void TestStrUtils::testParseFloat() {
   TEST_CASE("parseFloat");

   require(3.0f == StrUtils::parseFloat(string("3")), "1 digit positive integral");
   require(-5.0f == StrUtils::parseFloat(string("-5")), "1 digit negative integral");
   require(0.0f == StrUtils::parseFloat(string("0")), "zero");
   require(35.0f == StrUtils::parseFloat(string("35")), "2 digit positive integral");
   require(121.0f == StrUtils::parseFloat(string("121")), "3 digit positive integral");
   require(4096.0f == StrUtils::parseFloat(string("4096")), "4 digit positive integral");
   require(65535.0f == StrUtils::parseFloat(string("65535")),
           "5 digit positive integral");
   require(3.1415f == StrUtils::parseFloat(string("3.1415")), "positive float value");
   require(-1492.524f == StrUtils::parseFloat(string("-1492.524")),
           "negative float value");

   requireException("NumberFormatException", new ParseFloatRunner(""),
                    "empty string");
   requireException("NumberFormatException", new ParseFloatRunner("x"),
                    "letter");
   requireException("NumberFormatException", new ParseFloatRunner("y123"),
                    "leading char");
   requireException("NumberFormatException", new ParseFloatRunner("456t"),
                    "trailing char");
}

//******************************************************************************

void TestStrUtils::testParseDouble() {
   TEST_CASE("parseDouble");

   require(3.0 == StrUtils::parseDouble(string("3")), "1 digit positive integral");
   require(-5.0 == StrUtils::parseDouble(string("-5")), "1 digit negative integral");
   require(0.0 == StrUtils::parseDouble(string("0")), "zero");
   require(35.0 == StrUtils::parseDouble(string("35")), "2 digit positive integral");
   require(121.0 == StrUtils::parseDouble(string("121")), "3 digit positive integral");
   require(4096.0 == StrUtils::parseDouble(string("4096")), "4 digit positive integral");
   require(65535.0 == StrUtils::parseDouble(string("65535")),
           "5 digit positive integral");
   require(3.1415 == StrUtils::parseDouble(string("3.1415")), "positive double value");
   require(-1492.524 == StrUtils::parseDouble(string("-1492.524")),
           "negative double value");

   requireException("NumberFormatException", new ParseDoubleRunner(""),
                    "empty string");
   requireException("NumberFormatException", new ParseDoubleRunner("x"),
                    "letter");
   requireException("NumberFormatException", new ParseDoubleRunner("y123"),
                    "leading char");
   requireException("NumberFormatException", new ParseDoubleRunner("456t"),
                    "trailing char");
}

//******************************************************************************

void TestStrUtils::testToStringWithInt() {
   TEST_CASE("toStringWithInt");

   requireStringEquals(StrUtils::toString(2112), "2112", "positive value");
   requireStringEquals(StrUtils::toString(-57), "-57", "negative value");
   requireStringEquals(StrUtils::toString(0), "0", "zero");
}

//******************************************************************************

void TestStrUtils::testToStringWithLong() {
   TEST_CASE("toStringWithLong");

   requireStringEquals(StrUtils::toString(2112L), "2112", "positive value");
   requireStringEquals(StrUtils::toString(-57L), "-57", "negative value");
   requireStringEquals(StrUtils::toString(0L), "0", "zero");
}

//******************************************************************************

void TestStrUtils::testToStringWithULong() {
   TEST_CASE("toStringWithULong");

   requireStringEquals(StrUtils::toString(2112UL), "2112", "non-zero value");
   requireStringEquals(StrUtils::toString(0UL), "0", "zero");
}

//******************************************************************************

void TestStrUtils::testToStringWithLongLong() {
   TEST_CASE("toStringWithLongLong");

   requireStringEquals(StrUtils::toString(2112LL), "2112", "positive value");
   requireStringEquals(StrUtils::toString(-57LL), "-57", "negative value");
   requireStringEquals(StrUtils::toString(0LL), "0", "zero");
}

//******************************************************************************

void TestStrUtils::testStrip() {
   TEST_CASE("strip");

   const std::string target = "now is the time";
   std::string withSingleTrailingSpace = "now is the time ";
   std::string withSingleLeadingTrailingSpace = " now is the time ";
   std::string leadingAndTrailing = "   now is the time   ";
   std::string alreadyTrimmed = "now is the time";

   requireStringEquals(target, StrUtils::strip(withSingleTrailingSpace),
                       "single trailing space");
   requireStringEquals(target, StrUtils::strip(withSingleLeadingTrailingSpace),
                       "single leading and trailing space");
   requireStringEquals(target, StrUtils::strip(leadingAndTrailing),
                       "trimmed string has leading strip chars");
   requireStringEquals(target, StrUtils::strip(alreadyTrimmed),
                       "no alteration of already trimmed string");
}

//******************************************************************************

void TestStrUtils::testStripWithChar() {
   TEST_CASE("stripWithChar");

   const std::string target = "now is the time";
   std::string withSingleTrailingSpace = "now is the time ";
   std::string withSingleLeadingTrailingSpace = " now is the time ";
   std::string withSingleLeadingX = "xnow is the time";
   std::string withSingleTrailingX = "now is the timex";
   std::string withLeadingTrailingPunctuation = ",now is the time,";
   std::string withTrailingPunctuation = "now is the time,";
   std::string leadingAndTrailing = "...now is the time...";
   std::string alreadyTrimmed = "now is the time";

   requireStringEquals(target, StrUtils::strip(withSingleTrailingSpace, ' '),
                       "single trailing space");
   requireStringEquals(target, StrUtils::strip(withSingleLeadingTrailingSpace,
                                               ' '),
                       "single leading and trailing space");
   requireStringEquals(target, StrUtils::strip(withSingleLeadingX, 'x'),
                       "leading char");
   requireStringEquals(target, StrUtils::strip(withSingleTrailingX, 'x'),
                       "trailing char");
   requireStringEquals(target, StrUtils::strip(withTrailingPunctuation, ','),
                       "trailing punctuation");
   requireStringEquals(target, StrUtils::strip(withLeadingTrailingPunctuation,
                                               ','),
                       "leading and trailing punctuation");
   requireStringEquals(target, StrUtils::strip(leadingAndTrailing, '.'),
                       "leading and trailing strip chars");
   requireStringEquals(target, StrUtils::strip(alreadyTrimmed, ' '),
                       "no alteration of already trimmed string");
}

//******************************************************************************

void TestStrUtils::testStartsWith() {
   TEST_CASE("startsWith");

   const std::string haystack = "abcdefg";
   const std::string needle = "abc";
   const std::string razor = "xyz";
   const std::string upperNeedle = "ABC";

   require(StrUtils::startsWith(haystack, needle),
           "haystack contains needle at beginning");
   requireFalse(StrUtils::startsWith(haystack, razor),
                "haystack doesn't contain needle anywhere");
   requireFalse(StrUtils::startsWith(haystack, emptyString),
                "haystack doesn't start with empty string");
   requireFalse(StrUtils::startsWith(emptyString, needle),
                "empty haystack doesn't start with needle");
   requireFalse(StrUtils::startsWith(haystack, upperNeedle),
                "haystack doesn't start with upper needle");
}

//******************************************************************************

void TestStrUtils::testEndsWith() {
   TEST_CASE("endsWith");

   const std::string haystack = "abcdefg";
   const std::string needle = "efg";
   const std::string razor = "xyz";
   const std::string upperNeedle = "EFG";

   require(StrUtils::endsWith(haystack, needle),
           "haystack contains needle at end");
   requireFalse(StrUtils::endsWith(haystack, razor),
                "haystack doesn't contain needle anywhere");
   requireFalse(StrUtils::endsWith(haystack, emptyString),
                "haystack doesn't end with empty string");
   requireFalse(StrUtils::endsWith(emptyString, needle),
                "empty haystack doesn't end with needle");
   requireFalse(StrUtils::endsWith(haystack, upperNeedle),
                "haystack doesn't end with upper needle");
}

//******************************************************************************

void TestStrUtils::testStartsAndEndsWith() {
   TEST_CASE("startsAndEndsWith");

   const std::string surroundedBySpaces = "  in the beginning  ";
   const std::string spaceAtBeginning = "  in the beginning";
   const std::string spaceAtEnd = "in the beginning  ";
   const std::string spaceAtNeitherEnd = "in the beginning";
   const std::string space = " ";

   require(StrUtils::startsAndEndsWith(surroundedBySpaces, space),
           "haystack contains needle at start and end");
   requireFalse(StrUtils::startsAndEndsWith(spaceAtBeginning, space),
                "haystack contains needle at start only");
   requireFalse(StrUtils::startsAndEndsWith(spaceAtEnd, space),
                "haystack contains needle at end only");
   requireFalse(StrUtils::startsAndEndsWith(spaceAtNeitherEnd, space),
                "haystack doesn't contain needle at either end");
}

//******************************************************************************

void TestStrUtils::testContainsString() {
   TEST_CASE("containsString");

   const std::string haystack =
      "She said that it was he, and I said that it was she";
   const std::string She = "She";
   const std::string she = "she";
   const std::string he = "he";
   const std::string notThere = "continent";

   require(StrUtils::containsString(haystack, She), "haystack contains needle");
   require(StrUtils::containsString(haystack, she), "haystack contains needle");
   require(StrUtils::containsString(haystack, he), "haystack contains needle");
   requireFalse(StrUtils::containsString(haystack, notThere),
                "haystack does not contain needle");
   requireFalse(StrUtils::containsString(haystack, emptyString),
                "haystack doesn't contain empty string'");
   requireFalse(StrUtils::containsString(emptyString, haystack),
                "empty doesn't contain haystack'");
}

//******************************************************************************

void TestStrUtils::testToUpperCase() {
   TEST_CASE("toUpperCase");

   const std::string target = "NOW IS THE TIME";
   std::string source = "now is the time";

   StrUtils::toUpperCase(source);
   requireStringEquals(target, source, "all lower should convert to all upper");

   source = "Now Is The Time";
   StrUtils::toUpperCase(source);
   requireStringEquals(target, source, "mixed case should convert to all upper");

   source = target;
   StrUtils::toUpperCase(source);
   requireStringEquals(target, source,
                       "no alteration of already uppercase string");

   const std::string targetNonLetters = "1234;.,!";

   source = targetNonLetters;
   StrUtils::toUpperCase(source);
   requireStringEquals(targetNonLetters, source,
                       "no alteration of string not containing letters");
}

//******************************************************************************

void TestStrUtils::testToLowerCase() {
   TEST_CASE("toLowerCase");

   const std::string target = "now is the time";
   std::string source = "NOW IS THE TIME";

   StrUtils::toLowerCase(source);
   requireStringEquals(target, source, "all upper should convert to all lower");

   source = "Now Is The Time";
   StrUtils::toLowerCase(source);
   requireStringEquals(target, source, "mixed case should convert to all lower");

   source = target;
   StrUtils::toLowerCase(source);
   requireStringEquals(target, source,
                       "no alteration of already lowercase string");

   const std::string targetNonLetters = "1234;.,!";

   source = targetNonLetters;
   StrUtils::toLowerCase(source);
   requireStringEquals(targetNonLetters, source,
                       "no alteration of string not containing letters");
}

//******************************************************************************

void TestStrUtils::testReplaceAll() {
   TEST_CASE("replaceAll");

   const std::string source =
      "She said that it was he, and I said that it was she";
   const std::string target_she_She =
      "She said that it was he, and I said that it was She";
   const std::string target_She_she =
      "she said that it was he, and I said that it was she";
   const std::string target_She_He =
      "He said that it was he, and I said that it was she";
   const std::string target_he_she =
      "Sshe said that it was she, and I said that it was sshe";
   std::string target;

   const std::string She = "She";
   const std::string she = "she";
   const std::string He = "He";
   const std::string he = "he";
   const std::string notThere = "or";
   const std::string xyz = "xyz";

   target = source;
   requireStringEquals(target_she_She, StrUtils::replaceAll(target, she, She),
                       "replace 'she' with 'She'");

   target = source;
   requireStringEquals(target_She_she, StrUtils::replaceAll(target, She, she),
                       "replace 'She' with 'she'");

   target = source;
   requireStringEquals(target_She_He, StrUtils::replaceAll(target, She, He),
                       "replace 'She' with 'He'");

   target = source;
   requireStringEquals(target_he_she, StrUtils::replaceAll(target, he, she),
                       "replace 'he' with 'she'");

   target = source;
   requireStringEquals(target, StrUtils::replaceAll(target, notThere, xyz),
                       "no alteration of string with non-existent needle");
}

//******************************************************************************

void TestStrUtils::testStripInPlace() {
   TEST_CASE("stripInPlace");

   const std::string target = "now is the time";
   std::string withSingleTrailingSpace = "now is the time ";
   std::string withSingleLeadingTrailingSpace = " now is the time ";
   std::string withSingleLeadingX = "xnow is the time";
   std::string withSingleTrailingX = "now is the timex";
   std::string withLeadingTrailingPunctuation = ",now is the time,";
   std::string withTrailingPunctuation = "now is the time,";
   std::string leadingAndTrailing = "...now is the time...";
   std::string alreadyTrimmed = "now is the time";

   requireStringEquals(target, StrUtils::strip(withSingleTrailingSpace, ' '),
                       "single trailing space");
   requireStringEquals(target, StrUtils::strip(withSingleLeadingTrailingSpace,
                                               ' '),
                       "single leading and trailing space");
   requireStringEquals(target, StrUtils::strip(withSingleLeadingX, 'x'),
                       "leading char");
   requireStringEquals(target, StrUtils::strip(withSingleTrailingX, 'x'),
                       "trailing char");
   requireStringEquals(target, StrUtils::strip(withTrailingPunctuation, ','),
                       "trailing punctuation");
   requireStringEquals(target, StrUtils::strip(withLeadingTrailingPunctuation,
                                               ','),
                       "leading and trailing punctuation");
   requireStringEquals(target, StrUtils::strip(leadingAndTrailing, '.'),
                       "leading and trailing strip chars");
   requireStringEquals(target, StrUtils::strip(alreadyTrimmed, ' '),
                       "no alteration of already trimmed string");
}

//******************************************************************************

void TestStrUtils::testStripTrailing() {
   TEST_CASE("stripTrailing");

   const std::string target = "now is the time";
   std::string withSingleTrailingSpace = "now is the time ";
   std::string withSingleTrailingX = "now is the timex";
   std::string withTrailingPunctuation = "now is the time,";
   std::string leadingAndTrailing = "...now is the time...";
   std::string alreadyTrimmed = "now is the time";
   std::string onlyTrimChars = "xxxxxxxxxx";
   const std::string empty = "";

   requireStringEquals(target, StrUtils::stripTrailing(withSingleTrailingSpace,
                                                       ' '),
                       "single trailing space");
   requireStringEquals(target, StrUtils::stripTrailing(withSingleTrailingX,
                                                       'x'),
                       "trailing char");
   requireStringEquals(target, StrUtils::stripTrailing(withTrailingPunctuation,
                                                       ','),
                       "trailing punctuation");
   require(target != StrUtils::stripTrailing(leadingAndTrailing, '.'),
           "trimmed string has leading strip chars");
   requireStringEquals(target, StrUtils::stripTrailing(alreadyTrimmed, ' '),
                       "no alteration of already trimmed string");
   requireStringEquals(empty, StrUtils::stripTrailing(onlyTrimChars, 'x'),
                       "string containing only char to strip");
}

//******************************************************************************

void TestStrUtils::testStripLeading() {
   TEST_CASE("stripLeading");

   const std::string target = "now is the time";
   std::string withSingleLeadingSpace = " now is the time";
   std::string withSingleLeadingX = "xnow is the time";
   std::string withLeadingPunctuation = ",now is the time";
   std::string leadingAndTrailing = "...now is the time...";
   std::string alreadyTrimmed = "now is the time";

   requireStringEquals(target, StrUtils::stripLeading(withSingleLeadingSpace, ' '),
                       "single leading space");
   requireStringEquals(target, StrUtils::stripLeading(withSingleLeadingX, 'x'),
                       "leading char");
   requireStringEquals(target, StrUtils::stripLeading(withLeadingPunctuation, ','),
                       "leading punctuation");
   require(target != StrUtils::stripLeading(leadingAndTrailing, '.'),
           "trimmed string has trailing strip chars");
   requireStringEquals(target, StrUtils::stripLeading(alreadyTrimmed, ' '),
                       "no alteration of already trimmed string");
}

//******************************************************************************

void TestStrUtils::testTrimLeadingSpaces() {
   TEST_CASE("trimLeadingSpaces");

   const std::string target = "now is the time";
   std::string withSingleLeadingSpace = " now is the time";
   std::string withLeadingSpaces = "   now is the time";
   std::string leadingAndTrailing = "  now is the time  ";
   std::string alreadyTrimmed = "now is the time";

   requireStringEquals(target, StrUtils::trimLeadingSpaces(withSingleLeadingSpace),
                       "single leading space");
   requireStringEquals(target, StrUtils::trimLeadingSpaces(withLeadingSpaces),
                       "leading spaces");
   require(target != StrUtils::trimLeadingSpaces(leadingAndTrailing),
           "trimmed string has trailing spaces");
   requireStringEquals(target, StrUtils::trimLeadingSpaces(alreadyTrimmed),
                       "no alteration of already trimmed string");
}

//******************************************************************************

void TestStrUtils::testTrim() {
   TEST_CASE("trim");
   // trim is just alias for strip. strip tested elsewhere
}

//******************************************************************************

void TestStrUtils::testPadRight() {
   TEST_CASE("padRight");

   std::string startedEmpty;
   const std::string tenChars = "xxxxxxxxxx";
   StrUtils::padRight(startedEmpty, 'x', 10);
   requireStringEquals(tenChars, startedEmpty, "empty");

   std::string noPaddingNeeded = "xxxxxxxxxx";
   StrUtils::padRight(noPaddingNeeded, 'x', 10);
   requireStringEquals(tenChars, noPaddingNeeded, "no padding needed");

   std::string onePadCharNeeded = "...";
   const std::string fourChars = "....";
   StrUtils::padRight(onePadCharNeeded, '.', 4);
   requireStringEquals(fourChars, onePadCharNeeded, "one pad char needed");

   std::string threePadCharsNeeded = "888    ";
   const std::string spacePadded = "888       ";
   StrUtils::padRight(threePadCharsNeeded, ' ', 10);
   requireStringEquals(spacePadded, threePadCharsNeeded,
                       "three pad chars needed");
}

//******************************************************************************

void TestStrUtils::testPadLeft() {
   TEST_CASE("padLeft");

   std::string startedEmpty;
   const std::string tenChars = "xxxxxxxxxx";
   StrUtils::padLeft(startedEmpty, 'x', 10);
   requireStringEquals(tenChars, startedEmpty, "empty");

   std::string noPaddingNeeded = "xxxxxxxxxx";
   StrUtils::padLeft(noPaddingNeeded, 'x', 10);
   requireStringEquals(tenChars, noPaddingNeeded, "no padding needed");

   std::string onePadCharNeeded = "...";
   const std::string fourChars = "....";
   StrUtils::padLeft(onePadCharNeeded, '.', 4);
   requireStringEquals(fourChars, onePadCharNeeded, "one pad char needed");

   std::string threePadCharsNeeded = "888    ";
   const std::string spacePadded = "   888    ";
   StrUtils::padLeft(threePadCharsNeeded, ' ', 10);
   requireStringEquals(spacePadded, threePadCharsNeeded,
                       "three pad chars needed");
}

//******************************************************************************

void TestStrUtils::testMakeStringOfChar() {
   TEST_CASE("makeStringOfChar");

   requireStringEquals(std::string("xxx"),
                       StrUtils::makeStringOfChar('x', 3),
                       "simple construction");
   requireStringEquals(std::string(""),
                       StrUtils::makeStringOfChar('z', 0),
                       "zero-length");
}

//******************************************************************************

void TestStrUtils::testSplit() {
   TEST_CASE("split");

   std::vector<std::string> r;

   r = StrUtils::split(string("comma,separated,values"), ",");
   require(r.size() == 3, "comma separated values");

   r = StrUtils::split(string("/usr/local/bin"), "/");
   require(r.size() == 3, "leading delimiter");

   r = StrUtils::split(string("/usr/local/bin"), ":");
   require(r.size() == 1, "missing delimiter");

   r = StrUtils::split(string("abc:def:ghi:"), ":");
   require(r.size() == 3, "trailing delimiter");

   r = StrUtils::split(string("The-Who--Whos-Next--My-Wife"), "--");
   require(r.size() == 3, "repeating char delimiter");
}

//******************************************************************************


//******************************************************************************
//******************************************************************************

// The std::string_view overloads of parseFloat() and parseDouble() are a
// separate implementation from the std::string ones tested above (they use
// std::from_chars, or a strtod_l fallback where the standard library lacks the
// floating-point overloads). Every call below passes a std::string_view, so
// none of it can silently run the std::string overloads.

namespace {

// a closeness check for results of decimal-to-binary conversion: exact equality
// is only claimed where the value is exactly representable
template <typename T>
bool closeTo(T actual, double expected) {
   const double a = static_cast<double>(actual);
   if (a == expected) {
      return true;
   }
   const double tolerance = std::fabs(expected) *
                            static_cast<double>(std::numeric_limits<T>::epsilon()) * 2.0;
   return std::fabs(a - expected) <= tolerance;
}

template <typename T>
T parseView(std::string_view s);

template <>
float parseView<float>(std::string_view s) {
   return StrUtils::parseFloat(s);
}

template <>
double parseView<double>(std::string_view s) {
   return StrUtils::parseDouble(s);
}

template <typename T>
bool rejects(std::string_view s) {
   try {
      parseView<T>(s);
   } catch (const NumberFormatException&) {
      return true;
   } catch (...) {
      return false;
   }
   return false;
}

template <typename T>
bool accepts(std::string_view s) {
   try {
      parseView<T>(s);
   } catch (...) {
      return false;
   }
   return true;
}

const char* const INVALID_INPUTS[] = {
   "", "-", ".", "-.", "..", "1.2.3", "--1", "1-", "-1-",
   " 1", "1 ", " 1 ", "\t1", "1\n", "1 2",
   "+1", "+.5",
   "1e5", "1E5", "1e-3", "1E-3", "2.5e10", ".5e1",
   "nan", "NaN", "-nan", "inf", "-inf", "Inf", "infinity", "-infinity",
   "0x10", "0X1F", "0x1.8p3",
   "1,5", "3,1415", ",5", "5,",
   "456t", "y123", "12abc", "abc", "x", "1.5f", "1.5d", "--", "-+1"
};

template <typename T>
void checkFloatingParsing(poivre::TestSuite& t, const char* type) {
   const std::string name(type);

   // valid inputs
   struct Valid { const char* text; double value; };
   const Valid valid[] = {
      {"3", 3.0}, {"-5", -5.0}, {"0", 0.0}, {"35", 35.0}, {"65535", 65535.0},
      {"3.1415", 3.1415}, {"-1492.524", -1492.524}, {"1.", 1.0}, {".5", 0.5},
      {"-.5", -0.5}, {"00012", 12.0}, {"0.1", 0.1}, {"123456789", 123456789.0}
   };
   for (const Valid& v : valid) {
      t.require(closeTo<T>(parseView<T>(std::string_view(v.text)), v.value),
                name + " accepts \"" + v.text + "\"");
   }

   const T negativeZero = parseView<T>(std::string_view("-0"));
   t.require((negativeZero == 0) && std::signbit(negativeZero),
             name + " \"-0\" keeps its sign bit");
   const T negativeZeroDecimal = parseView<T>(std::string_view("-0.0"));
   t.require((negativeZeroDecimal == 0) && std::signbit(negativeZeroDecimal),
             name + " \"-0.0\" keeps its sign bit");
   const T positiveZero = parseView<T>(std::string_view("0"));
   t.require((positiveZero == 0) && !std::signbit(positiveZero),
             name + " \"0\" is positive zero");

   // the view's length is respected, wherever the view sits in its string
   t.require(closeTo<T>(parseView<T>(std::string_view("12345").substr(0, 3)), 123.0),
             name + " view \"123\" of \"12345\"");
   t.require(closeTo<T>(parseView<T>(std::string_view("1.5abc").substr(0, 3)), 1.5),
             name + " valid prefix followed by invalid characters outside the view");
   t.require(closeTo<T>(parseView<T>(std::string_view("x-7.25y").substr(1, 5)), -7.25),
             name + " view in the middle of a string");
   const std::string backing("42xyz");
   t.require(closeTo<T>(parseView<T>(std::string_view(backing.data(), 2)), 42.0),
             name + " view shorter than its std::string");
   // no terminator inside the view, and a digit right behind it: a parser that
   // ignored the length would return 123457
   const char unterminated[] = {'1', '2', '3', '4', '5', '7', '\0'};
   t.require(closeTo<T>(parseView<T>(std::string_view(unterminated, 5)), 12345.0),
             name + " view ends before the next digit");
   t.require(rejects<T>(std::string_view("12x").substr(0, 3)),
             name + " an invalid character inside the view is rejected");
   t.require(rejects<T>(std::string_view("123").substr(0, 0)),
             name + " empty view of a numeric string");

   // invalid inputs
   for (const char* text : INVALID_INPUTS) {
      t.require(rejects<T>(std::string_view(text)),
                name + " rejects \"" + text + "\"");
   }
   // an embedded NUL is not a digit
   t.require(rejects<T>(std::string_view("1\0" "2", 3)),
             name + " rejects an embedded NUL");

   // range, written without exponents because the pre-filter does not accept them
   const std::string manyNines(400, '9');
   const std::string underflow = "0." + std::string(400, '0') + "1";
   t.require(rejects<T>(std::string_view(manyNines)), name + " overflow (400 digits)");
   t.require(rejects<T>(std::string_view("-" + manyNines)), name + " negative overflow");
   t.require(rejects<T>(std::string_view(underflow)), name + " underflow to zero");
   t.require(rejects<T>(std::string_view("-" + underflow)), name + " negative underflow to zero");
}

}  // namespace

//******************************************************************************

POIVRE_TEST_CASE(TestStrUtils, testParseFloatStringView) {
   checkFloatingParsing<float>(*this, "float");

   // float limits
   const std::string fifty(50, '9');
   require(rejects<float>(std::string_view(fifty)),
           "float: 50 digits is beyond the float range");
   require(accepts<double>(std::string_view(fifty)),
           "the same 50 digits fit a double");

   // FLT_MAX is 340282346638528859811704183484516925440 exactly
   const float maxFloat =
      StrUtils::parseFloat(std::string_view("340282346638528859811704183484516925440"));
   require(maxFloat == std::numeric_limits<float>::max(), "float: FLT_MAX");
   require(closeTo<float>(StrUtils::parseFloat(std::string_view("3000000000000000000000000000000000000")),
                          3.0e36),
           "float: a value of the order of 1e36");

   // representable subnormal float: 1e-40 < FLT_MIN (about 1.17e-38)
   const float subnormal =
      StrUtils::parseFloat(std::string_view("0." + std::string(39, '0') + "1"));
   require((subnormal > 0) && std::isfinite(subnormal) &&
           (subnormal < std::numeric_limits<float>::min()),
           "float: a representable subnormal value is accepted");
}

//******************************************************************************

POIVRE_TEST_CASE(TestStrUtils, testParseDoubleStringView) {
   checkFloatingParsing<double>(*this, "double");

   // a value near the top of the double range, written out in full
   const std::string nearMax = "1" + std::string(300, '0');
   require(closeTo<double>(StrUtils::parseDouble(std::string_view(nearMax)), 1.0e300),
           "double: 1 followed by 300 zeros");

   // representable subnormal double: 1e-311 < DBL_MIN (about 2.2e-308)
   const double subnormal =
      StrUtils::parseDouble(std::string_view("0." + std::string(310, '0') + "1"));
   require((subnormal > 0) && std::isfinite(subnormal) &&
           (subnormal < std::numeric_limits<double>::min()),
           "double: a representable subnormal value is accepted");
}

//******************************************************************************

// The conversion must not depend on the locale's decimal separator. Where a
// locale whose decimal separator is a comma can be selected, parse under it;
// the selection is limited to the calling thread (uselocale() on POSIX,
// _configthreadlocale() on Windows), so other threads and tests are not
// affected. If there is no such locale, the check is skipped and says so.
POIVRE_TEST_CASE(TestStrUtils, testParseFloatingLocaleIndependent) {
   require(closeTo<double>(StrUtils::parseDouble(std::string_view("3.1415")), 3.1415),
           "baseline: \"3.1415\" in the unchanged locale");

   bool commaLocaleActive = false;

#ifdef _WIN32
   const int previousThreadMode = _configthreadlocale(_ENABLE_PER_THREAD_LOCALE);
   const char* const names[] = {"de-DE", "fr-FR", "de_DE", "fr_FR"};
   for (const char* name : names) {
      if (std::setlocale(LC_NUMERIC, name) != nullptr) {
         const struct lconv* conv = std::localeconv();
         if ((conv != nullptr) && (std::string(conv->decimal_point) == ",")) {
            commaLocaleActive = true;
            break;
         }
      }
   }
   if (commaLocaleActive) {
      require(closeTo<double>(StrUtils::parseDouble(std::string_view("3.1415")), 3.1415),
              "double \"3.1415\" under a comma-decimal locale");
      require(closeTo<float>(StrUtils::parseFloat(std::string_view("-1492.524")), -1492.524),
              "float \"-1492.524\" under a comma-decimal locale");
      require(rejects<double>(std::string_view("3,1415")),
              "double \"3,1415\" is still rejected under a comma-decimal locale");
      require(rejects<float>(std::string_view("3,1415")),
              "float \"3,1415\" is still rejected under a comma-decimal locale");
   }
   std::setlocale(LC_NUMERIC, "C");
   if (previousThreadMode != -1) {
      _configthreadlocale(previousThreadMode);
   }
#else
   const char* const names[] = {"de_DE.UTF-8", "de_DE.utf8", "fr_FR.UTF-8", "fr_FR.utf8",
                                "de_DE", "fr_FR", "it_IT.UTF-8", "nl_NL.UTF-8"};
   for (const char* name : names) {
      locale_t commaLocale = newlocale(LC_ALL_MASK, name, static_cast<locale_t>(nullptr));
      if (commaLocale == static_cast<locale_t>(nullptr)) {
         continue;
      }
      const locale_t previous = uselocale(commaLocale);
      const struct lconv* conv = std::localeconv();
      const bool isComma = (conv != nullptr) && (std::string(conv->decimal_point) == ",");
      if (isComma) {
         commaLocaleActive = true;
         require(closeTo<double>(StrUtils::parseDouble(std::string_view("3.1415")), 3.1415),
                 std::string("double \"3.1415\" under the thread-local locale ") + name);
         require(closeTo<float>(StrUtils::parseFloat(std::string_view("-1492.524")), -1492.524),
                 std::string("float \"-1492.524\" under the thread-local locale ") + name);
         require(rejects<double>(std::string_view("3,1415")),
                 std::string("double \"3,1415\" is still rejected under ") + name);
         require(rejects<float>(std::string_view("3,1415")),
                 std::string("float \"3,1415\" is still rejected under ") + name);
      }
      uselocale(previous);
      freelocale(commaLocale);
      if (isComma) {
         break;
      }
   }
#endif

   if (!commaLocaleActive) {
      std::printf("NOTE: testParseFloatingLocaleIndependent: no comma-decimal locale "
                  "is installed; the locale-independence check was SKIPPED\n");
   }
}
