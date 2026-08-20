// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestDynamicLibrary.h"
#include "DynamicLibrary.h"
#include "BasicException.h"

using namespace chaudiere;

static const std::string LIBC_NAME = "libc.so.6";
static const std::string LIBC_SYMBOL = "printf";

//******************************************************************************

TestDynamicLibrary::TestDynamicLibrary() :
   poivre::TestSuite("TestDynamicLibrary") {
}

//******************************************************************************

void TestDynamicLibrary::runTests() {
   testDefaultConstructor();
   testConstructorWithLibraryName();
   testConstructorWithInvalidLibraryName();
   testOpen();
   testOpenInvalid();
   testClose();
   testResolve();
   testResolveWithoutOpen();
}

//******************************************************************************

void TestDynamicLibrary::testDefaultConstructor() {
   TEST_CASE("testDefaultConstructor");

   DynamicLibrary lib;
   require(nullptr == lib.resolve(LIBC_SYMBOL), "a default-constructed (unopened) library should resolve nothing");
}

//******************************************************************************

void TestDynamicLibrary::testConstructorWithLibraryName() {
   TEST_CASE("testConstructorWithLibraryName");

   DynamicLibrary lib(LIBC_NAME);
   require(nullptr != lib.resolve(LIBC_SYMBOL), "a library opened via the constructor should resolve a known symbol");
}

//******************************************************************************

void TestDynamicLibrary::testConstructorWithInvalidLibraryName() {
   TEST_CASE("testConstructorWithInvalidLibraryName");

   class BadLibraryCreator : public poivre::Runnable {
   public:
      void run() override {
         DynamicLibrary lib("no_such_library_xyz.so");
      }
   };

   requireException("BasicException", new BadLibraryCreator());
}

//******************************************************************************

void TestDynamicLibrary::testOpen() {
   TEST_CASE("testOpen");

   DynamicLibrary lib;
   require(lib.open(LIBC_NAME), "open should succeed for a valid library name");
   require(nullptr != lib.resolve(LIBC_SYMBOL), "a known symbol should resolve after a successful open");
}

//******************************************************************************

void TestDynamicLibrary::testOpenInvalid() {
   TEST_CASE("testOpenInvalid");

   DynamicLibrary lib;
   requireFalse(lib.open("no_such_library_xyz.so"), "open should fail for a non-existent library");
}

//******************************************************************************

void TestDynamicLibrary::testClose() {
   TEST_CASE("testClose");

   DynamicLibrary lib(LIBC_NAME);
   require(nullptr != lib.resolve(LIBC_SYMBOL), "sanity check: symbol should resolve before close");

   lib.close();
   require(nullptr == lib.resolve(LIBC_SYMBOL), "resolve should fail after close");

   // close should be idempotent
   lib.close();
   require(true, "calling close a second time should not crash");
}

//******************************************************************************

void TestDynamicLibrary::testResolve() {
   TEST_CASE("testResolve");

   DynamicLibrary lib(LIBC_NAME);
   void* symbol = lib.resolve(LIBC_SYMBOL);
   require(nullptr != symbol, "resolve should return a non-null pointer for a known, exported symbol");

   void* missingSymbol = lib.resolve("no_such_symbol_xyz");
   require(nullptr == missingSymbol, "resolve should return null for a symbol that doesn't exist in the library");
}

//******************************************************************************

void TestDynamicLibrary::testResolveWithoutOpen() {
   TEST_CASE("testResolveWithoutOpen");

   DynamicLibrary lib;
   require(nullptr == lib.resolve(LIBC_SYMBOL), "resolve should return null (not fall back to a global symbol search) when no library has been opened");
}

//******************************************************************************
