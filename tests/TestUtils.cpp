// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <memory>
#include <string.h>

#include "TestUtils.h"
#include "Utils.h"
#include "ByteBuffer.h"

using namespace chaudiere;

//******************************************************************************

TestUtils::TestUtils() :
   poivre::TestSuite("TestUtils") {
}

//******************************************************************************

void TestUtils::runTests() {
   testWriteFile();
   testReadFile();
   testReadFileNonExistent();
   testMd5ForFile();
}

//******************************************************************************

void TestUtils::testWriteFile() {
   TEST_CASE("testWriteFile");

   const std::string filePath = getTempFile();
   ByteBuffer buffer(std::string("hello, world"));
   require(Utils::WriteFile(filePath, buffer), "WriteFile should succeed for a valid path and buffer");

   std::unique_ptr<ByteBuffer> readBack(Utils::ReadFile(filePath));
   require(nullptr != readBack, "the file just written should be readable back");
   require(12 == readBack->size(), "read-back size should match what was written");

   deleteFile(filePath);
}

//******************************************************************************

void TestUtils::testReadFile() {
   TEST_CASE("testReadFile");

   const std::string filePath = getTempFile();
   ByteBuffer buffer(std::string("round trip content"));
   Utils::WriteFile(filePath, buffer);

   std::unique_ptr<ByteBuffer> readBack(Utils::ReadFile(filePath));
   require(nullptr != readBack, "ReadFile should return a non-null buffer for an existing file");
   require(18 == readBack->size(), "read-back size should match the file's content length");
   require(0 == ::memcmp("round trip content", readBack->const_data(), 18), "read-back content should match what was written");

   deleteFile(filePath);
}

//******************************************************************************

void TestUtils::testReadFileNonExistent() {
   TEST_CASE("testReadFileNonExistent");

   ByteBuffer* buffer = Utils::ReadFile("/no/such/path/does-not-exist.bin");
   require(nullptr == buffer, "ReadFile should return null for a non-existent file");
}

//******************************************************************************

void TestUtils::testMd5ForFile() {
   TEST_CASE("testMd5ForFile");

   // Md5ForFile is a documented TODO in Utils.cpp (unimplemented) and
   // currently always returns an empty string regardless of input
   const std::string filePath = getTempFile();
   ByteBuffer buffer(std::string("content"));
   Utils::WriteFile(filePath, buffer);

   require(Utils::Md5ForFile(filePath).empty(), "Md5ForFile is unimplemented and currently always returns an empty string");

   deleteFile(filePath);
}

//******************************************************************************
