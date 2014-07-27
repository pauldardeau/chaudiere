// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestStringTokenizer.h"
#include "StringTokenizer.h"

//******************************************************************************

TestStringTokenizer::TestStringTokenizer() :
   TestSuite("TestStringTokenizer")
{
}

//******************************************************************************

void TestStringTokenizer::runTests()
{
   testConstructor();
   testConstructorWithDelimiter();
   testHasMoreTokens();
   testNextToken();
   testCountTokens();
}

//******************************************************************************

void TestStringTokenizer::testConstructor()
{
   TEST_CASE("testConstructor");
   
   StringTokenizer stLetters("a b c");
   require(stLetters.countTokens() == 3, "3 tokens");

   StringTokenizer stNames("John Paul George Ringo");
   require(stNames.countTokens() == 4, "4 tokens");
}

//******************************************************************************

void TestStringTokenizer::testConstructorWithDelimiter()
{
   TEST_CASE("testConstructorWithDelimiter");

   StringTokenizer stSpace("a b c", " ");
   require(stSpace.countTokens() == 3, "3 tokens");

   StringTokenizer stComma("a,b,c", ",");
   require(stComma.countTokens() == 3, "3 tokens");
}

//******************************************************************************

void TestStringTokenizer::testHasMoreTokens()
{
   TEST_CASE("testHasMoreTokens");

   StringTokenizer st("a b c");
   require(st.hasMoreTokens(), "3 tokens");
   st.nextToken();
   require(st.hasMoreTokens(), "3 tokens");
   st.nextToken();
   require(st.hasMoreTokens(), "3 tokens");
   st.nextToken();
   requireFalse(st.hasMoreTokens(), "3 tokens");
}

//******************************************************************************

void TestStringTokenizer::testNextToken()
{
   TEST_CASE("testNextToken");

   std::string token;
   StringTokenizer st("a b c");
   require(st.hasMoreTokens(), "3 tokens");
   token = st.nextToken();
   requireStringEquals("a", token, "first token");
   require(st.hasMoreTokens(), "3 tokens");
   token = st.nextToken();
   requireStringEquals("b", token, "second token");
   require(st.hasMoreTokens(), "3 tokens");
   token = st.nextToken();
   requireStringEquals("c", token, "third token");
}

//******************************************************************************

void TestStringTokenizer::testCountTokens()
{
   TEST_CASE("testCountTokens");

   StringTokenizer st("a b c");
   require(3 == st.countTokens(), "3 tokens");
}

//******************************************************************************

