// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestPthreadsMutex.h"
#include "PthreadsMutex.h"

//******************************************************************************

TestPthreadsMutex::TestPthreadsMutex() :
   TestSuite("TestMutex")
{
}

//******************************************************************************

void TestPthreadsMutex::runTests()
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

void TestPthreadsMutex::testConstructor()
{
   TEST_CASE("testConstructor");
   
   PthreadsMutex mutex;
   require(mutex.haveValidMutex(), "constructor should result in valid mutex");
}

//******************************************************************************

void TestPthreadsMutex::testConstructorWithName()
{
   TEST_CASE("testConstructorWithName");

   PthreadsMutex mutex("testMutex");
   require(mutex.haveValidMutex(), "constructor with name should result in valid mutex");
}

//******************************************************************************

void TestPthreadsMutex::testLock()
{
   TEST_CASE("testLock");
   
   PthreadsMutex mutex;
   require(mutex.lock(), "should be able to lock");
   require(mutex.isLocked(), "isLocked should return true");
}

//******************************************************************************

void TestPthreadsMutex::testUnlock()
{
   TEST_CASE("testUnlock");

   PthreadsMutex mutex;
   require(mutex.lock(), "should be able to lock");
   require(mutex.isLocked(), "isLocked should return true");
   require(mutex.unlock(), "should be able to unlock");
   requireFalse(mutex.isLocked(), "isLocked should return false after unlock");
}

//******************************************************************************

void TestPthreadsMutex::testHaveValidMutex()
{
   TEST_CASE("testHaveValidMutex");
   
   PthreadsMutex mutex;
   require(mutex.haveValidMutex(), "constructor should result in valid mutex");
}

//******************************************************************************

void TestPthreadsMutex::testGetPlatformPrimitive()
{
   TEST_CASE("testGetPlatformPrimitive");

   PthreadsMutex mutex;
   require(mutex.haveValidMutex(), "constructor should result in valid mutex");
}

//******************************************************************************

void TestPthreadsMutex::testGetName()
{
   TEST_CASE("testGetName");
   
   const std::string name = "testMutex";
   PthreadsMutex mutex(name);
   requireStringEquals(name, mutex.getName(), "name should match value given to ctor");
}

//******************************************************************************

void TestPthreadsMutex::testIsLocked()
{
   TEST_CASE("testIsLocked");
   
   PthreadsMutex mutex;
   if (mutex.lock()) {
      require(mutex.isLocked(), "isLock should return true when locked");
      if (mutex.unlock()) {
         requireFalse(mutex.isLocked(), "isLock should return false when unlocked");
      }
   }
}

//******************************************************************************

