// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <cstdio>
#include <cstdlib>

#include "TestSuite.h"


//******************************************************************************

TestSuite::TestSuite(const std::string& suiteName) :
   m_suiteName(suiteName),
   m_numTestsExecuted(0),
   m_numFailures(0)
{
}

//******************************************************************************

TestSuite::TestSuite(const TestSuite& copy) :
   m_suiteName(copy.m_suiteName),
   m_numTestsExecuted(copy.m_numTestsExecuted),
   m_numFailures(copy.m_numFailures)
{
}

//******************************************************************************

TestSuite::~TestSuite()
{
}

//******************************************************************************

TestSuite& TestSuite::operator=(const TestSuite& copy)
{
   if (this == &copy) {
      return *this;
   }
   
   m_suiteName = copy.m_suiteName;
   m_numTestsExecuted = copy.m_numTestsExecuted;
   m_numFailures = copy.m_numFailures;
   
   return *this;
}

//******************************************************************************

const std::string& TestSuite::getName() const
{
   return m_suiteName;
}

//******************************************************************************

void TestSuite::setup()
{
}

//******************************************************************************

void TestSuite::tearDown()
{
}

//******************************************************************************

void TestSuite::suiteSetup()
{
}

//******************************************************************************

void TestSuite::suiteTearDown()
{
}

//******************************************************************************

void TestSuite::run()
{
   suiteSetup();
   runTests();
   suiteTearDown();
}

//******************************************************************************

std::string TestSuite::getTempFile() const
{
   char fileTemplate[] = "/tmp/fileXXXXXX.test";
   const int fd = ::mkstemp(fileTemplate);
   if (-1 != fd) {
      return std::string(fileTemplate);
   } else {
      return std::string("");
   }
}

//******************************************************************************

bool TestSuite::deleteFile(const std::string& filePath)
{
   const int rc = remove(filePath.c_str());
   return (0 == rc);
}

//******************************************************************************

void TestSuite::startingTestCase(const TestCase& testCase)
{
   std::printf("starting test case %s\n", testCase.getName().c_str());
   setup();
}

//******************************************************************************

void TestSuite::endingTestCase(const TestCase& testCase)
{
   tearDown();
   std::printf("ending test case %s\n", testCase.getName().c_str());
}

//******************************************************************************

void TestSuite::require(bool expression, const std::string& testDesc)
{
   if (!expression) {
      ++m_numFailures;
      std::printf("*** failure: expected true, got false (%s)\n",
             testDesc.c_str());
   }
   
   ++m_numTestsExecuted;
}

//******************************************************************************

void TestSuite::requireFalse(bool expression, const std::string& testDesc)
{
   if (expression) {
      ++m_numFailures;
      std::printf("*** failure: expected false, got true (%s)\n",
             testDesc.c_str());
   }
   
   ++m_numTestsExecuted;
}

//******************************************************************************

void TestSuite::requireStringEquals(const std::string& expected,
                                    const std::string& actual,
                                    const std::string& testDesc)
{
   if (expected != actual) {
      ++m_numFailures;
      std::printf("*** failure: expected '%s', actual='%s' (%s)\n",
             expected.c_str(),
             actual.c_str(),
             testDesc.c_str());
   }

   ++m_numTestsExecuted;
}

//******************************************************************************

void TestSuite::requireNonEmptyString(const std::string& actual,
                                      const std::string& testDesc)
{
   if (actual.empty()) {
      ++m_numFailures;
      std::printf("*** failure: expected non-empty string (%s)\n",
             testDesc.c_str());
   }
   
   ++m_numTestsExecuted;
}

//******************************************************************************

