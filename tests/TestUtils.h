// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTUTILS_H
#define CHAUDIERE_TESTUTILS_H

#include "TestSuite.h"

namespace chaudiere
{

class TestUtils : public poivre::TestSuite
{
protected:
   void runTests();

   void testWriteFile();
   void testReadFile();
   void testReadFileNonExistent();
   void testMd5ForFile();

public:
   TestUtils();

};

}

#endif
