// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <string>

#include "TestStrUtils.h"
#include "StrUtils.h"

static const std::string emptyString = "";

//******************************************************************************

TestStrUtils::TestStrUtils() :
   TestSuite("TestStrUtils")
{
}

//******************************************************************************

void TestStrUtils::runTests()
{
   testStrip();
   testStripWithChar();
   testStartsWith();
   testEndsWith();
   testContainsString();
   testToUpperCase();
   testToLowerCase();
   testReplaceAll();
   testStripInPlace();
   testStripTrailing();
   testStripLeading();
   testTrimLeadingSpaces();
}

//******************************************************************************

void TestStrUtils::testStrip()
{
   TEST_CASE("strip");

   //static std::string strip(const std::string& s);
   const std::string target = "now is the time";
   std::string withSingleTrailingSpace = "now is the time ";
   std::string withSingleLeadingTrailingSpace = " now is the time ";
   std::string leadingAndTrailing = "   now is the time   ";
   std::string alreadyTrimmed = "now is the time";
   
   requireStringEquals(target, StrUtils::strip(withSingleTrailingSpace), "single trailing space");
   requireStringEquals(target, StrUtils::strip(withSingleLeadingTrailingSpace), "single leading and trailing space");
   requireStringEquals(target, StrUtils::strip(leadingAndTrailing), "trimmed string has leading strip chars");
   requireStringEquals(target, StrUtils::strip(alreadyTrimmed), "no alteration of already trimmed string");
}

//******************************************************************************

void TestStrUtils::testStripWithChar()
{
   TEST_CASE("stripWithChar");

   //static std::string strip(const std::string& s, char strip);
   const std::string target = "now is the time";
   std::string withSingleTrailingSpace = "now is the time ";
   std::string withSingleLeadingTrailingSpace = " now is the time ";
   std::string withSingleLeadingX = "xnow is the time";
   std::string withSingleTrailingX = "now is the timex";
   std::string withLeadingTrailingPunctuation = ",now is the time,";
   std::string withTrailingPunctuation = "now is the time,";
   std::string leadingAndTrailing = "...now is the time...";
   std::string alreadyTrimmed = "now is the time";
   
   requireStringEquals(target, StrUtils::strip(withSingleTrailingSpace, ' '), "single trailing space");
   requireStringEquals(target, StrUtils::strip(withSingleLeadingTrailingSpace, ' '), "single leading and trailing space");
   requireStringEquals(target, StrUtils::strip(withSingleLeadingX, 'x'), "leading char");
   requireStringEquals(target, StrUtils::strip(withSingleTrailingX, 'x'), "trailing char");
   requireStringEquals(target, StrUtils::strip(withTrailingPunctuation, ','), "trailing punctuation");
   requireStringEquals(target, StrUtils::strip(withLeadingTrailingPunctuation, ','), "leading and trailing punctuation");
   requireStringEquals(target, StrUtils::strip(leadingAndTrailing, '.'), "leading and trailing strip chars");
   requireStringEquals(target, StrUtils::strip(alreadyTrimmed, ' '), "no alteration of already trimmed string");
}

//******************************************************************************

void TestStrUtils::testStartsWith()
{
   TEST_CASE("startsWith");
   
   //static bool startsWith(const std::string& haystack, const std::string& needle);
   const std::string haystack    = "abcdefg";
   const std::string needle      = "abc";
   const std::string razor       = "xyz";
   const std::string upperNeedle = "ABC";
   
   require(StrUtils::startsWith(haystack, needle), "haystack contains needle at beginning");
   requireFalse(StrUtils::startsWith(haystack, razor), "haystack doesn't contain needle anywhere");
   requireFalse(StrUtils::startsWith(haystack, emptyString), "haystack doesn't start with empty string");
   requireFalse(StrUtils::startsWith(emptyString, needle), "empty haystack doesn't start with needle");
   requireFalse(StrUtils::startsWith(haystack, upperNeedle), "haystack doesn't start with upper needle");
}

//******************************************************************************

void TestStrUtils::testEndsWith()
{
   TEST_CASE("endsWith");
   
   //static bool endsWith(const std::string& haystack, const std::string& needle);
   const std::string haystack    = "abcdefg";
   const std::string needle      = "efg";
   const std::string razor       = "xyz";
   const std::string upperNeedle = "EFG";
   
   require(StrUtils::endsWith(haystack, needle), "haystack contains needle at end");
   requireFalse(StrUtils::endsWith(haystack, razor), "haystack doesn't contain needle anywhere");
   requireFalse(StrUtils::endsWith(haystack, emptyString), "haystack doesn't end with empty string");
   requireFalse(StrUtils::endsWith(emptyString, needle), "empty haystack doesn't end with needle");
   requireFalse(StrUtils::endsWith(haystack, upperNeedle), "haystack doesn't end with upper needle");
}

//******************************************************************************

void TestStrUtils::testContainsString()
{
   TEST_CASE("containsString");
   
   //static bool containsString(const std::string& haystack, const std::string& needle);
   const std::string haystack = "She said that it was he, and I said that it was she";
   const std::string She = "She";
   const std::string she = "she";
   const std::string he = "he";
   const std::string notThere = "continent";
   
   require(StrUtils::containsString(haystack, She), "haystack contains needle");
   require(StrUtils::containsString(haystack, she), "haystack contains needle");
   require(StrUtils::containsString(haystack, he), "haystack contains needle");
   requireFalse(StrUtils::containsString(haystack, notThere), "haystack does not contain needle");
}

//******************************************************************************

void TestStrUtils::testToUpperCase()
{
   TEST_CASE("toUpperCase");
   
   //static void toUpperCase(std::string& s);
   const std::string target = "NOW IS THE TIME";
   std::string source = "now is the time";
   
   StrUtils::toUpperCase(source);
   requireStringEquals(target, source, "all lower should convert to all upper");
   
   source = "Now Is The Time";
   StrUtils::toUpperCase(source);
   requireStringEquals(target, source, "mixed case should convert to all upper");

   source = target;
   StrUtils::toUpperCase(source);
   requireStringEquals(target, source, "no alteration of already uppercase string");
   
   const std::string targetNonLetters = "1234;.,!";
   
   source = targetNonLetters;
   StrUtils::toUpperCase(source);
   requireStringEquals(targetNonLetters, source, "no alteration of string not containing letters");
}

//******************************************************************************

void TestStrUtils::testToLowerCase()
{
   TEST_CASE("toLowerCase");
   
   //static void toLowerCase(std::string& s);
   const std::string target = "now is the time";
   std::string source = "NOW IS THE TIME";
   
   StrUtils::toLowerCase(source);
   requireStringEquals(target, source, "all upper should convert to all lower");
   
   source = "Now Is The Time";
   StrUtils::toLowerCase(source);
   requireStringEquals(target, source, "mixed case should convert to all lower");
   
   source = target;
   StrUtils::toLowerCase(source);
   requireStringEquals(target, source, "no alteration of already lowercase string");
   
   const std::string targetNonLetters = "1234;.,!";
   
   source = targetNonLetters;
   StrUtils::toLowerCase(source);
   requireStringEquals(targetNonLetters, source, "no alteration of string not containing letters");
}

//******************************************************************************

void TestStrUtils::testReplaceAll()
{
   TEST_CASE("replaceAll");
   
   //static std::string& replaceAll(std::string& s, const std::string& searchFor, const std::string& replaceWith);
   const std::string source         = "She said that it was he, and I said that it was she";
   const std::string target_she_She = "She said that it was he, and I said that it was She";
   const std::string target_She_she = "she said that it was he, and I said that it was she";
   const std::string target_She_He  = "He said that it was he, and I said that it was she";
   const std::string target_he_she  = "Sshe said that it was she, and I said that it was sshe";
   std::string target;
   
   const std::string She = "She";
   const std::string she = "she";
   const std::string He = "He";
   const std::string he = "he";
   const std::string notThere = "or";
   const std::string xyz = "xyz";

   target = source;
   requireStringEquals(target_she_She, StrUtils::replaceAll(target, she, She), "replace 'she' with 'She'");
   
   target = source;
   requireStringEquals(target_She_she, StrUtils::replaceAll(target, She, she), "replace 'She' with 'she'");

   target = source;
   requireStringEquals(target_She_He, StrUtils::replaceAll(target, She, He), "replace 'She' with 'He'");

   target = source;
   requireStringEquals(target_he_she, StrUtils::replaceAll(target, he, she), "replace 'he' with 'she'");

   target = source;
   requireStringEquals(target, StrUtils::replaceAll(target, notThere, xyz), "no alteration of string with non-existent needle");
}

//******************************************************************************

void TestStrUtils::testStripInPlace()
{
   TEST_CASE("stripInPlace");

   //static std::string& strip(std::string& s, char strip=' ');
   const std::string target = "now is the time";
   std::string withSingleTrailingSpace = "now is the time ";
   std::string withSingleLeadingTrailingSpace = " now is the time ";
   std::string withSingleLeadingX = "xnow is the time";
   std::string withSingleTrailingX = "now is the timex";
   std::string withLeadingTrailingPunctuation = ",now is the time,";
   std::string withTrailingPunctuation = "now is the time,";
   std::string leadingAndTrailing = "...now is the time...";
   std::string alreadyTrimmed = "now is the time";
   
   requireStringEquals(target, StrUtils::strip(withSingleTrailingSpace, ' '), "single trailing space");
   requireStringEquals(target, StrUtils::strip(withSingleLeadingTrailingSpace, ' '), "single leading and trailing space");
   requireStringEquals(target, StrUtils::strip(withSingleLeadingX, 'x'), "leading char");
   requireStringEquals(target, StrUtils::strip(withSingleTrailingX, 'x'), "trailing char");
   requireStringEquals(target, StrUtils::strip(withTrailingPunctuation, ','), "trailing punctuation");
   requireStringEquals(target, StrUtils::strip(withLeadingTrailingPunctuation, ','), "leading and trailing punctuation");
   requireStringEquals(target, StrUtils::strip(leadingAndTrailing, '.'), "leading and trailing strip chars");
   requireStringEquals(target, StrUtils::strip(alreadyTrimmed, ' '), "no alteration of already trimmed string");
}

//******************************************************************************

void TestStrUtils::testStripTrailing()
{
   TEST_CASE("stripTrailing");

   //static std::string& stripTrailing(std::string& s, char strip);
   const std::string target = "now is the time";
   std::string withSingleTrailingSpace = "now is the time ";
   std::string withSingleTrailingX = "now is the timex";
   std::string withTrailingPunctuation = "now is the time,";
   std::string leadingAndTrailing = "...now is the time...";
   std::string alreadyTrimmed = "now is the time";
   
   requireStringEquals(target, StrUtils::stripTrailing(withSingleTrailingSpace, ' '), "single trailing space");
   requireStringEquals(target, StrUtils::stripTrailing(withSingleTrailingX, 'x'), "trailing char");
   requireStringEquals(target, StrUtils::stripTrailing(withTrailingPunctuation, ','), "trailing punctuation");
   require(target != StrUtils::stripTrailing(leadingAndTrailing, '.'), "trimmed string has leading strip chars");
   requireStringEquals(target, StrUtils::stripTrailing(alreadyTrimmed, ' '), "no alteration of already trimmed string");
}

//******************************************************************************

void TestStrUtils::testStripLeading()
{
   TEST_CASE("stripLeading");

   //static std::string& stripLeading(std::string& s, char strip);
   const std::string target = "now is the time";
   std::string withSingleLeadingSpace = " now is the time";
   std::string withSingleLeadingX = "xnow is the time";
   std::string withLeadingPunctuation = ",now is the time";
   std::string leadingAndTrailing = "...now is the time...";
   std::string alreadyTrimmed = "now is the time";
   
   requireStringEquals(target, StrUtils::stripLeading(withSingleLeadingSpace, ' '), "single leading space");
   requireStringEquals(target, StrUtils::stripLeading(withSingleLeadingX, 'x'), "leading char");
   requireStringEquals(target, StrUtils::stripLeading(withLeadingPunctuation, ','), "leading punctuation");
   require(target != StrUtils::stripLeading(leadingAndTrailing, '.'), "trimmed string has trailing strip chars");
   requireStringEquals(target, StrUtils::stripLeading(alreadyTrimmed, ' '), "no alteration of already trimmed string");
}

//******************************************************************************

void TestStrUtils::testTrimLeadingSpaces()
{
   TEST_CASE("trimLeadingSpaces");

   //static std::string& trimLeadingSpaces(std::string& s);
   const std::string target = "now is the time";
   std::string withSingleLeadingSpace = " now is the time";
   std::string withLeadingSpaces = "   now is the time";
   std::string leadingAndTrailing = "  now is the time  ";
   std::string alreadyTrimmed = "now is the time";
   
   requireStringEquals(target, StrUtils::trimLeadingSpaces(withSingleLeadingSpace), "single leading space");
   requireStringEquals(target, StrUtils::trimLeadingSpaces(withLeadingSpaces), "leading spaces");
   require(target != StrUtils::trimLeadingSpaces(leadingAndTrailing), "trimmed string has trailing spaces");
   requireStringEquals(target, StrUtils::trimLeadingSpaces(alreadyTrimmed), "no alteration of already trimmed string");
}

//******************************************************************************

