// Copyright Paul Dardeau, SwampBits LLC 2015
// BSD License

#include "TestAutoPointer.h"
#include "AutoPointer.h"

using namespace chaudiere;

namespace {

class TrackedObject {
public:
   explicit TrackedObject(bool& destroyedFlag) :
      value(42),
      m_destroyedFlag(destroyedFlag) {
      m_destroyedFlag = false;
   }

   ~TrackedObject() {
      m_destroyedFlag = true;
   }

   int value;

private:
   bool& m_destroyedFlag;
};

}

//******************************************************************************

TestAutoPointer::TestAutoPointer() :
   poivre::TestSuite("TestAutoPointer") {
}

//******************************************************************************

void TestAutoPointer::runTests() {
   testConstructor();
   testOperatorArrow();
   testAssign();
   testHaveObject();
   testDestructor();
}

//******************************************************************************

void TestAutoPointer::testConstructor() {
   TEST_CASE("testConstructor");

   bool destroyed = false;
   {
      poivre::AutoPointer<TrackedObject> ap(new TrackedObject(destroyed));
      require(ap.haveObject(), "constructing with a non-null object should result in haveObject() true");
   }
   require(destroyed, "the owned object should be destroyed when the AutoPointer goes out of scope");
}

//******************************************************************************

void TestAutoPointer::testOperatorArrow() {
   TEST_CASE("testOperatorArrow");

   bool destroyed = false;
   poivre::AutoPointer<TrackedObject> ap(new TrackedObject(destroyed));
   require(42 == ap->value, "operator-> should provide access to the owned object's members");
}

//******************************************************************************

void TestAutoPointer::testAssign() {
   TEST_CASE("testAssign");

   bool firstDestroyed = false;
   bool secondDestroyed = false;
   poivre::AutoPointer<TrackedObject> ap(new TrackedObject(firstDestroyed));

   ap.assign(new TrackedObject(secondDestroyed));
   require(firstDestroyed, "assign should delete the previously-owned object");
   requireFalse(secondDestroyed, "assign should not delete the newly-assigned object");
   require(ap.haveObject(), "haveObject should be true after assign with a non-null object");
   require(42 == ap->value, "the AutoPointer should now provide access to the newly-assigned object");
}

//******************************************************************************

void TestAutoPointer::testHaveObject() {
   TEST_CASE("testHaveObject");

   poivre::AutoPointer<TrackedObject> apNull(nullptr);
   requireFalse(apNull.haveObject(), "haveObject should be false when constructed with nullptr");

   bool destroyed = false;
   poivre::AutoPointer<TrackedObject> apWithObject(new TrackedObject(destroyed));
   require(apWithObject.haveObject(), "haveObject should be true when constructed with a non-null object");
}

//******************************************************************************

void TestAutoPointer::testDestructor() {
   TEST_CASE("testDestructor");

   bool destroyed = false;
   {
      poivre::AutoPointer<TrackedObject> ap(new TrackedObject(destroyed));
      requireFalse(destroyed, "object should not be destroyed while the AutoPointer is still in scope");
   }
   require(destroyed, "object should be destroyed once the AutoPointer goes out of scope");
}

//******************************************************************************
