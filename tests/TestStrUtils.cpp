// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <string.h>
#include <string>

#include "TestStrUtils.h"
#include "StrUtils.h"

static const std::string emptyString = "";

using namespace chaudiere;
using namespace poivre;

//******************************************************************************

TestStrUtils::TestStrUtils() :
   TestSuite("TestStrUtils") {
}

//******************************************************************************

void TestStrUtils::runTests() {
   // parsing
   testParseInt();
   testParseLong();
   testParseFloat();
   testParseDouble();

   // toString
   testToStringWithInt();
   testToStringWithLong();
   testToStringWithULong();

   // strip
   testStrip();
   testStripWithChar();

   testStartsWith();
   testEndsWith();
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

   // gzip
   //TODO: testGzipCompress();
   //TODO: testGzipDecompress();

   // makeStringOfChar
   testMakeStringOfChar();

   // split
   testSplit();
}

//******************************************************************************

class ParseRunner : public Runnable {
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

   require(3 == StrUtils::parseInt("3"), "1 digit positive integer");
   require(-5 == StrUtils::parseInt("-5"), "1 digit negative integer");
   require(0 == StrUtils::parseInt("0"), "zero");
   require(35 == StrUtils::parseInt("35"), "2 digit positive integer");
   require(121 == StrUtils::parseInt("121"), "3 digit positive integer");
   require(4096 == StrUtils::parseInt("4096"), "4 digit positive integer");
   require(65535 == StrUtils::parseInt("65535"), "5 digit positive integer");

   // try largest (absolute value) signed value for 32 bits
   require(2147483647 == StrUtils::parseInt("2147483647"),
           "large positive int");
   require(-2147483648UL == StrUtils::parseInt("-2147483648"),
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

   require(3L == StrUtils::parseLong("3"), "1 digit positive long");
   require(-5L == StrUtils::parseLong("-5"), "1 digit negative long");
   require(0L == StrUtils::parseLong("0"), "zero");
   require(35L == StrUtils::parseLong("35"), "2 digit positive long");
   require(121L == StrUtils::parseLong("121"), "3 digit positive long");
   require(4096L == StrUtils::parseLong("4096"), "4 digit positive long");
   require(65535L == StrUtils::parseLong("65535"), "5 digit positive long");

   // try largest (absolute value) signed value for 32 bits
   require(2147483647L == StrUtils::parseLong("2147483647"),
           "large positive long");
   require(-2147483648UL == StrUtils::parseLong("-2147483648"),
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

void TestStrUtils::testParseFloat() {
   TEST_CASE("parseFloat");

   require(3.0f == StrUtils::parseFloat("3"), "1 digit positive integral");
   require(-5.0f == StrUtils::parseFloat("-5"), "1 digit negative integral");
   require(0.0f == StrUtils::parseFloat("0"), "zero");
   require(35.0f == StrUtils::parseFloat("35"), "2 digit positive integral");
   require(121.0f == StrUtils::parseFloat("121"), "3 digit positive integral");
   require(4096.0f == StrUtils::parseFloat("4096"), "4 digit positive integral");
   require(65535.0f == StrUtils::parseFloat("65535"),
           "5 digit positive integral");
   require(3.1415f == StrUtils::parseFloat("3.1415"), "positive float value");
   require(-1492.524f == StrUtils::parseFloat("-1492.524"),
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

   require(3.0 == StrUtils::parseDouble("3"), "1 digit positive integral");
   require(-5.0 == StrUtils::parseDouble("-5"), "1 digit negative integral");
   require(0.0 == StrUtils::parseDouble("0"), "zero");
   require(35.0 == StrUtils::parseDouble("35"), "2 digit positive integral");
   require(121.0 == StrUtils::parseDouble("121"), "3 digit positive integral");
   require(4096.0 == StrUtils::parseDouble("4096"), "4 digit positive integral");
   require(65535.0 == StrUtils::parseDouble("65535"),
           "5 digit positive integral");
   require(3.1415 == StrUtils::parseDouble("3.1415"), "positive double value");
   require(-1492.524 == StrUtils::parseDouble("-1492.524"),
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

void PopulateCompressionTestString(char* buffer, int buffer_len) {
   char ch;
   int char_count = 40;
   int offset = 0;

   ch = 'a';
   while ((offset + char_count) < buffer_len) {
      memset(buffer+offset, ch, char_count);
      if (ch == 'z') {
         ch = 'a';
      } else {
         ch += 1;
      }
      offset += char_count;
   }
}

//*****************************************************************************
//
/*
void TestStrUtils::testGzipCompress() {
   TEST_CASE("gzipCompress");

   char buffer[4096];
   PopulateCompressionTestString(buffer, 4096);
   std::string original(buffer);
   std::string compressed = StrUtils::gzipCompress(original);
   require(compressed.length() > 0, "compressed not empty");
   require(compressed.length() < original.length(),
           "compressed len < uncompressed len");
}
*/
//******************************************************************************
/*
void TestStrUtils::testGzipDecompress() {
   TEST_CASE("gzipDecompress");

   char buffer[4096];
   PopulateCompressionTestString(buffer, 4096);
   std::string original(buffer);
   std::string compressed = StrUtils::gzipCompress(original);
   std::string uncompressed = StrUtils::gzipDecompress(compressed);
   requireStringEquals(original, uncompressed, "uncompressed match original");
}
*/
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

   r = StrUtils::split("comma,separated,values", ",");
   require(r.size() == 3, "comma separated values");

   r = StrUtils::split("/usr/local/bin", "/");
   require(r.size() == 3, "leading delimiter");

   r = StrUtils::split("/usr/local/bin", ":");
   require(r.size() == 1, "missing delimiter");

   r = StrUtils::split("abc:def:ghi:", ":");
   require(r.size() == 3, "trailing delimiter");
}

//******************************************************************************

