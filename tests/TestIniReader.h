// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef TESTINIREADER_H
#define TESTINIREADER_H

#include "TestSuite.h"


class TestIniReader : public TestSuite
{
private:
   std::string m_filePath;

protected:
   void runTests();
   void setupSuite();
   void tearDownSuite();
   
   void testConstructor();
   void testReadSection();
   void testGetSectionKeyValue();
   void testHasSection();
   
public:
   TestIniReader();

};

#endif
