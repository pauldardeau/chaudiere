// Copyright Paul Dardeau, SwampBits LLC 2015
// BSD License

#include <string.h>

#include "TestByteBuffer.h"
#include "ByteBuffer.h"

using namespace chaudiere;

//******************************************************************************

TestByteBuffer::TestByteBuffer() :
   TestSuite("TestByteBuffer") {
}

//******************************************************************************

void TestByteBuffer::testDefaultConstructor() {
   ByteBuffer b;
}

//******************************************************************************

void TestByteBuffer::testSizeConstructor() {
   std::size_t buffSize = 256;
   ByteBuffer b(buffSize);
   require(buffSize == b.size(), "matching size");
   require(nullptr != b.data(), "non-null buffer pointer");
}

//******************************************************************************

void TestByteBuffer::testCopyConstructor() {
   std::size_t buffSize = 1024;
   void* raw = ::malloc(buffSize);
   ::memset(raw, 'x', buffSize);
   ByteBuffer b;
   b.take(raw, buffSize);
   ByteBuffer b2(b);
   void* b2Raw = b2.data();
   require(buffSize == b2.size(), "size after copy constructor");
   require(0 == memcmp(raw, b2Raw, buffSize), "matching data contents after copy constructor");
}

//******************************************************************************

void TestByteBuffer::testAssignmentOperator() {
   std::size_t buffSize = 1024;
   void* raw = ::malloc(buffSize);
   ::memset(raw, 'x', buffSize);
   ByteBuffer b;
   b.take(raw, buffSize);
   ByteBuffer b2;
   b2 = b;
   void* b2Raw = b2.data();
   require(buffSize == b2.size(), "size after assignment operator");
   require(0 == memcmp(raw, b2Raw, buffSize), "matching data contents after assignment operator");
}

//******************************************************************************

void TestByteBuffer::testTake() {
   std::size_t buffSize = 1024;
   void* raw = ::malloc(buffSize);
   ::memset(raw, 'x', buffSize);
   ByteBuffer b;
   b.take(raw, buffSize);
   require(buffSize == b.size(), "size should adjustment after take");
   require(raw == b.data(), "matching buffer after take");
}

//******************************************************************************

void TestByteBuffer::testRelease() {
   std::size_t buffSize = 1024;
   void* raw = ::malloc(buffSize);
   ::memset(raw, 'x', buffSize);
   ByteBuffer b;
   b.take(raw, buffSize);
   void* raw2 = b.release();
   require(raw == raw2, "release should match prior take");
}

//******************************************************************************

void TestByteBuffer::testData() {
   std::size_t buffSize = 1024;
   void* raw = ::malloc(buffSize);
   ::memset(raw, 'x', buffSize);
   ByteBuffer b;
   b.take(raw, buffSize);
   require(raw == b.data(), "matching buffer take");
}

//******************************************************************************

void TestByteBuffer::testSize() {
   std::size_t buffSize = 1024;
   void* raw = ::malloc(buffSize);
   ::memset(raw, 'x', buffSize);
   ByteBuffer b;
   b.take(raw, buffSize);
   require(buffSize == b.size(), "matching buffer size after take");
}

//******************************************************************************

void TestByteBuffer::testClear() {
   std::size_t buffSize = 1024;
   void* raw = ::malloc(buffSize);
   ::memset(raw, 'x', buffSize);
   ByteBuffer b;
   b.take(raw, buffSize);
   b.clear();
   require(0 == b.size(), "zero size after clear");
   require(nullptr == b.data(), "null pointer after clear");
}

//******************************************************************************

void TestByteBuffer::runTests() {
   testDefaultConstructor();
   testSizeConstructor();
   testCopyConstructor();
   testAssignmentOperator();
   testTake();
   testRelease();
   testData();
   testSize();
   testClear();
}

//******************************************************************************

