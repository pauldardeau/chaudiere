// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTMUTEXLOCK_H
#define CHAUDIERE_TESTMUTEXLOCK_H

#include "TestSuite.h"

namespace chaudiere
{

class TestMutexLock : public poivre::TestSuite
{
protected:
   void runTests();

   void testConstructorLocksMutex();
   void testConstructorWithName();
   void testDestructorUnlocksMutex();
   void testUnlock();
   void testUnlockThenDestroy();

public:
   TestMutexLock();

};

}

#endif
