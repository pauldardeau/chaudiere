// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <unistd.h>

#include "TestDynamicLibrary.h"
#include "DynamicLibrary.h"
#include "BasicException.h"

using namespace chaudiere;

#ifndef CHAUDIERE_DYNAMIC_LIBRARY_FIXTURE_PATH
#error "CHAUDIERE_DYNAMIC_LIBRARY_FIXTURE_PATH is set by tests/CMakeLists.txt"
#endif

// The shared library built from fixtures/DynamicLibraryFixture.cpp, by absolute
// path (so no library search path is involved), and the one function it exports
static const std::string FIXTURE_NAME = CHAUDIERE_DYNAMIC_LIBRARY_FIXTURE_PATH;
static const std::string FIXTURE_SYMBOL = "chaudiere_test_fixture_add";

// A symbol that is in the global symbol namespace of every POSIX process (the C
// library is linked into the test executable). The tests that expect nothing to
// resolve use it, because a lookup that wrongly fell back to a process-wide
// search would find it; the fixture's function, which is only visible through
// its own handle, would not show that.
static const std::string GLOBAL_SYMBOL = "printf";

typedef int (*FixtureAddFunction)(int, int);

//******************************************************************************

TestDynamicLibrary::TestDynamicLibrary() :
   poivre::TestSuite("TestDynamicLibrary") {
}

//******************************************************************************

void TestDynamicLibrary::runTests() {
   testFixtureIsAvailable();
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

void TestDynamicLibrary::testFixtureIsAvailable() {
   TEST_CASE("testFixtureIsAvailable");

   // the other tests rely on the fixture; if it is missing or the path is
   // wrong, say so here rather than leave it to their failures
   require(!FIXTURE_NAME.empty() && '/' == FIXTURE_NAME[0], "the fixture should be named by an absolute path, so that no library search path is involved");
   require(0 == ::access(FIXTURE_NAME.c_str(), R_OK), "the fixture shared library should have been built and be readable");
}

//******************************************************************************

void TestDynamicLibrary::testDefaultConstructor() {
   TEST_CASE("testDefaultConstructor");

   DynamicLibrary lib;
   require(nullptr == lib.resolve(GLOBAL_SYMBOL), "a default-constructed (unopened) library should resolve nothing");
}

//******************************************************************************

void TestDynamicLibrary::testConstructorWithLibraryName() {
   TEST_CASE("testConstructorWithLibraryName");

   DynamicLibrary lib(FIXTURE_NAME);
   require(nullptr != lib.resolve(FIXTURE_SYMBOL), "a library opened via the constructor should resolve a known symbol");
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
   require(lib.open(FIXTURE_NAME), "open should succeed for a valid library name");
   require(nullptr != lib.resolve(FIXTURE_SYMBOL), "a known symbol should resolve after a successful open");
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

   DynamicLibrary lib(FIXTURE_NAME);
   require(nullptr != lib.resolve(FIXTURE_SYMBOL), "sanity check: symbol should resolve before close");

   lib.close();
   require(nullptr == lib.resolve(FIXTURE_SYMBOL), "resolve should fail after close");
   require(nullptr == lib.resolve(GLOBAL_SYMBOL), "resolve should fail after close, not fall back to a global symbol search");

   // close should be idempotent
   lib.close();
   require(true, "calling close a second time should not crash");
}

//******************************************************************************

void TestDynamicLibrary::testResolve() {
   TEST_CASE("testResolve");

   DynamicLibrary lib(FIXTURE_NAME);
   void* symbol = lib.resolve(FIXTURE_SYMBOL);
   require(nullptr != symbol, "resolve should return a non-null pointer for a known, exported symbol");

   // the library is still open here, so the function can be called
   if (nullptr != symbol) {
      FixtureAddFunction add = reinterpret_cast<FixtureAddFunction>(symbol);
      require(5 == add(2, 3), "the resolved function should compute 2 + 3");
      require(0 == add(-4, 4), "the resolved function should compute -4 + 4");
   }

   void* missingSymbol = lib.resolve("no_such_symbol_xyz");
   require(nullptr == missingSymbol, "resolve should return null for a symbol that doesn't exist in the library");
}

//******************************************************************************

void TestDynamicLibrary::testResolveWithoutOpen() {
   TEST_CASE("testResolveWithoutOpen");

   DynamicLibrary lib;
   require(nullptr == lib.resolve(GLOBAL_SYMBOL), "resolve should return null (not fall back to a global symbol search) when no library has been opened");
}

//******************************************************************************
