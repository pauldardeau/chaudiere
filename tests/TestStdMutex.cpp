// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestStdMutex.h"
#include "StdMutex.h"

//******************************************************************************

TestStdMutex::TestStdMutex() :
   TestSuite("TestMutex")
{
}

//******************************************************************************

void TestStdMutex::runTests()
{
   testConstructor();
   testConstructorWithName();
   testLock();
   testUnlock();
   testHaveValidMutex();
   testGetPlatformPrimitive();
   testGetName();
   testIsLocked();
}

//******************************************************************************

void TestStdMutex::testConstructor()
{
   TEST_CASE("testConstructor");
   
   StdMutex mutex;
   require(mutex.haveValidMutex(), "constructor should result in valid mutex");
}

//******************************************************************************

void TestStdMutex::testConstructorWithName()
{
   TEST_CASE("testConstructorWithName");
   
   StdMutex mutex("testMutex");
   require(mutex.haveValidMutex(), "constructor with name should result in valid mutex");
}

//******************************************************************************

void TestStdMutex::testLock()
{
   TEST_CASE("testLock");
   
   StdMutex mutex;
   require(mutex.lock(), "should be able to lock");
   require(mutex.isLocked(), "isLocked should return true");
}

//******************************************************************************

void TestStdMutex::testUnlock()
{
   TEST_CASE("testUnlock");
   
   StdMutex mutex;
   require(mutex.lock(), "should be able to lock");
   require(mutex.isLocked(), "isLocked should return true");
   require(mutex.unlock(), "should be able to unlock");
   requireFalse(mutex.isLocked(), "isLocked should return false after unlock");
}

//******************************************************************************

void TestStdMutex::testHaveValidMutex()
{
   TEST_CASE("testHaveValidMutex");
   
   StdMutex mutex;
   require(mutex.haveValidMutex(), "constructor should result in valid mutex");
}

//******************************************************************************

void TestStdMutex::testGetPlatformPrimitive()
{
   TEST_CASE("testGetPlatformPrimitive");
   
   StdMutex mutex;
   require(mutex.haveValidMutex(), "constructor should result in valid mutex");
}

//******************************************************************************

void TestStdMutex::testGetName()
{
   TEST_CASE("testGetName");
   
   const std::string name = "testMutex";
   StdMutex mutex(name);
   requireStringEquals(name, mutex.getName(), "name should match value given to ctor");
}

//******************************************************************************

void TestStdMutex::testIsLocked()
{
   TEST_CASE("testIsLocked");
   
   StdMutex mutex;
   if (mutex.lock()) {
      require(mutex.isLocked(), "isLock should return true when locked");
      if (mutex.unlock()) {
         requireFalse(mutex.isLocked(), "isLock should return false when unlocked");
      }
   }
}

//******************************************************************************

