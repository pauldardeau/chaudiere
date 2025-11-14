// Copyright Paul Dardeau, SwampBits LLC 2017
// BSD License

#ifndef CHAUDIERE_TESTCASE_H
#define CHAUDIERE_TESTCASE_H

#include <string>

#include "AbstractTestCase.h"
#include "AbstractTestSuite.h"

namespace chaudiere
{

class TestCase : public AbstractTestCase
{
private:
   AbstractTestSuite& m_testSuite;
   std::string m_testCaseName;

   TestCase(const TestCase& copy);
   TestCase& operator=(const TestCase& copy);


public:
   TestCase(AbstractTestSuite& testSuite, const std::string& testCaseName) :
      m_testSuite(testSuite),
      m_testCaseName(testCaseName) {

      m_testSuite.startingTestCase(*this);
   }

   virtual ~TestCase() {
      m_testSuite.endingTestCase(*this);
   }

   const std::string& getName() const {
      return m_testCaseName;
   }
};

}

#endif
