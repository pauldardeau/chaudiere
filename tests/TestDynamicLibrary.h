// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTDYNAMICLIBRARY_H
#define CHAUDIERE_TESTDYNAMICLIBRARY_H

#include "TestSuite.h"

namespace chaudiere
{

class TestDynamicLibrary : public poivre::TestSuite
{
protected:
   void runTests();

   void testDefaultConstructor();
   void testConstructorWithLibraryName();
   void testConstructorWithInvalidLibraryName();
   void testOpen();
   void testOpenInvalid();
   void testClose();
   void testResolve();
   void testResolveWithoutOpen();

public:
   TestDynamicLibrary();

};

}

#endif
