// Copyright Paul Dardeau, SwampBits LLC 2015
// BSD License

#include <string.h>

#include "TestCharBuffer.h"
#include "CharBuffer.h"

using namespace chaudiere;

//******************************************************************************

TestCharBuffer::TestCharBuffer() :
   poivre::TestSuite("TestCharBuffer") {
}

//******************************************************************************

void TestCharBuffer::runTests() {
   testDefaultConstructor();
   testConstructorWithSize();
   testNullAt();
   testAllocateBuffer();
   testEnsureCapacityWhenEmpty();
   testEnsureCapacityGrowing();
   testEnsureCapacityAlreadySufficient();
   testData();
   testSize();
}

//******************************************************************************

void TestCharBuffer::testDefaultConstructor() {
   TEST_CASE("testDefaultConstructor");

   CharBuffer cb;
   require(nullptr == cb.data(), "default-constructed buffer should have no data pointer");
   require(0 == cb.size(), "default-constructed buffer should have size 0");
}

//******************************************************************************

void TestCharBuffer::testConstructorWithSize() {
   TEST_CASE("testConstructorWithSize");

   CharBuffer cb(10);
   require(nullptr != cb.data(), "buffer constructed with a size should have an allocated data pointer");
   require(10 == cb.size(), "buffer size should match the constructor argument");
   require(0 == cb.data()[0], "buffer contents should be zero-initialized");
   require(0 == cb.data()[9], "buffer contents should be zero-initialized");
}

//******************************************************************************

void TestCharBuffer::testNullAt() {
   TEST_CASE("testNullAt");

   CharBuffer cb(5);
   ::memset(cb.data(), 'x', 5);
   cb.nullAt(2);
   require('\0' == cb.data()[2], "nullAt should write a null terminator at the given offset");
   require('x' == cb.data()[1], "nullAt should not affect other bytes");

   // an out-of-range offset should be safely ignored
   cb.nullAt(100);
   require(true, "nullAt with an out-of-range offset should not crash");
}

//******************************************************************************

void TestCharBuffer::testAllocateBuffer() {
   TEST_CASE("testAllocateBuffer");

   CharBuffer cb;
   cb.allocateBuffer(8);
   require(nullptr != cb.data(), "allocateBuffer should result in a non-null data pointer");
   require(8 == cb.size(), "size should reflect the newly allocated buffer");
   require(0 == cb.data()[0], "newly allocated buffer contents should be zero-initialized");
}

//******************************************************************************

void TestCharBuffer::testEnsureCapacityWhenEmpty() {
   TEST_CASE("testEnsureCapacityWhenEmpty");

   CharBuffer cb;
   cb.ensureCapacity(6);
   require(nullptr != cb.data(), "ensureCapacity on an empty buffer should allocate one");
   require(6 == cb.size(), "size should match the requested capacity");
}

//******************************************************************************

void TestCharBuffer::testEnsureCapacityGrowing() {
   TEST_CASE("testEnsureCapacityGrowing");

   CharBuffer cb(4);
   ::memcpy(cb.data(), "abcd", 4);

   cb.ensureCapacity(10);
   require(10 == cb.size(), "size should grow to accommodate the requested capacity");
   require(0 == ::memcmp("abcd", cb.data(), 4), "existing content should be preserved after growing");
}

//******************************************************************************

void TestCharBuffer::testEnsureCapacityAlreadySufficient() {
   TEST_CASE("testEnsureCapacityAlreadySufficient");

   CharBuffer cb(10);
   char* originalPtr = cb.data();

   cb.ensureCapacity(5);
   require(originalPtr == cb.data(), "ensureCapacity should not reallocate when the buffer is already large enough");
   require(10 == cb.size(), "size should remain unchanged when the buffer is already large enough");
}

//******************************************************************************

void TestCharBuffer::testData() {
   TEST_CASE("testData");

   CharBuffer cb(5);
   cb.data()[0] = 'A';
   require('A' == cb.data()[0], "data should provide direct read/write access to the buffer");
}

//******************************************************************************

void TestCharBuffer::testSize() {
   TEST_CASE("testSize");

   CharBuffer cb(7);
   require(7 == cb.size(), "size should return the buffer's current capacity");
}

//******************************************************************************
