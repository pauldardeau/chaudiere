// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestInvalidKeyException.h"
#include "InvalidKeyException.h"

//******************************************************************************

TestInvalidKeyException::TestInvalidKeyException() :
   TestSuite("TestInvalidKeyException")
{
}

//******************************************************************************

void TestInvalidKeyException::runTests()
{
   testConstructor();
   testCopyConstructor();
   testMoveConstructor();
   testAssignmentCopy();
   testAssignmentMove();
   testGetKey();
}

//******************************************************************************

void TestInvalidKeyException::testConstructor()
{
   TEST_CASE("testConstructor");
   
   InvalidKeyException ike("TestKey");
   requireStringEquals("TestKey", ike.getKey(), "key should match ctor arg");
   requireNonEmptyString(ike.whatString(), "whatString should not be empty");
}

//******************************************************************************

void TestInvalidKeyException::testCopyConstructor()
{
   TEST_CASE("testCopyConstructor");

   InvalidKeyException ike("TestKey");
   InvalidKeyException ikeCopy(ike);
   requireStringEquals("TestKey", ikeCopy.getKey(), "key should match ctor arg");
   requireNonEmptyString(ikeCopy.whatString(), "whatString should not be empty");
}

//******************************************************************************

void TestInvalidKeyException::testMoveConstructor()
{
   TEST_CASE("testMoveConstructor");
   
}

//******************************************************************************

void TestInvalidKeyException::testAssignmentCopy()
{
   TEST_CASE("testAssignmentCopy");

   InvalidKeyException ike("TestKey");
   InvalidKeyException ikeCopy("SomeOtherKey");
   ikeCopy = ike;
   requireStringEquals("TestKey", ikeCopy.getKey(), "key should match ctor arg");
   requireNonEmptyString(ikeCopy.whatString(), "whatString should not be empty");
}

//******************************************************************************

void TestInvalidKeyException::testAssignmentMove()
{
   TEST_CASE("testAssignmentMove");
   
}

//******************************************************************************

void TestInvalidKeyException::testGetKey()
{
   TEST_CASE("testGetKey");
   
   InvalidKeyException ike("TestKey");
   requireStringEquals("TestKey", ike.getKey(), "key should match ctor arg");
}

//******************************************************************************
