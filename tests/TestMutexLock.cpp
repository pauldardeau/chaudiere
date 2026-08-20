// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestMutexLock.h"
#include "MutexLock.h"
#include "PthreadsMutex.h"

using namespace chaudiere;

//******************************************************************************

TestMutexLock::TestMutexLock() :
   poivre::TestSuite("TestMutexLock") {
}

//******************************************************************************

void TestMutexLock::runTests() {
   testConstructorLocksMutex();
   testConstructorWithName();
   testDestructorUnlocksMutex();
   testUnlock();
   testUnlockThenDestroy();
}

//******************************************************************************

void TestMutexLock::testConstructorLocksMutex() {
   TEST_CASE("testConstructorLocksMutex");

   PthreadsMutex mutex;
   requireFalse(mutex.isLocked(), "sanity check: mutex should not be locked before MutexLock is constructed");
   {
      MutexLock lock(mutex);
      require(mutex.isLocked(), "constructing a MutexLock should lock the given mutex");
   }
   requireFalse(mutex.isLocked(), "mutex should be unlocked once the MutexLock goes out of scope");
}

//******************************************************************************

void TestMutexLock::testConstructorWithName() {
   TEST_CASE("testConstructorWithName");

   PthreadsMutex mutex;
   MutexLock lock(mutex, "myLockName");
   require(mutex.isLocked(), "constructing a named MutexLock should also lock the given mutex");
}

//******************************************************************************

void TestMutexLock::testDestructorUnlocksMutex() {
   TEST_CASE("testDestructorUnlocksMutex");

   PthreadsMutex mutex;
   {
      MutexLock lock(mutex);
   }
   requireFalse(mutex.isLocked(), "the destructor should unlock the mutex");
}

//******************************************************************************

void TestMutexLock::testUnlock() {
   TEST_CASE("testUnlock");

   PthreadsMutex mutex;
   MutexLock lock(mutex);
   require(mutex.isLocked(), "sanity check: mutex should be locked");
   lock.unlock();
   requireFalse(mutex.isLocked(), "unlock() should release the mutex early");
}

//******************************************************************************

void TestMutexLock::testUnlockThenDestroy() {
   TEST_CASE("testUnlockThenDestroy");

   PthreadsMutex mutex;
   {
      MutexLock lock(mutex);
      lock.unlock();
   }
   requireFalse(mutex.isLocked(), "mutex should remain unlocked after an explicit unlock() followed by destruction");
   require(true, "destruction after an explicit unlock() should not double-unlock or crash");
}

//******************************************************************************
