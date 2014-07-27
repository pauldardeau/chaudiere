// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestCase.h"
#include "TestSuite.h"


//******************************************************************************

TestCase::TestCase(TestSuite& testSuite, const std::string& testCaseName) :
   m_testSuite(testSuite),
   m_testCaseName(testCaseName)
{
   m_testSuite.startingTestCase(*this);
}

//******************************************************************************

TestCase::~TestCase()
{
   m_testSuite.endingTestCase(*this);
}

//******************************************************************************

const std::string& TestCase::getName() const
{
   return m_testCaseName;
}

//******************************************************************************

