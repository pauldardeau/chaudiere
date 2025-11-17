// Copyright Paul Dardeau, SwampBits LLC 2017
// BSD License

#ifndef CHAUDIERE_TESTSUITE_H
#define CHAUDIERE_TESTSUITE_H

#include <stdio.h>
#include <string.h>
#include <string>

#include "TestMacros.h"
#include "AbstractTestCase.h"
#include "AbstractTestSuite.h"
#include "AutoPointer.h"
#include "BasicException.h"
#include "Runnable.h"


namespace chaudiere
{

class TestSuite : public AbstractTestSuite
{
private:
   std::string m_suiteName;
   int m_numTestsExecuted;
   int m_numFailures;


protected:
   virtual void setup() {}
   virtual void tearDown() {}

   virtual void suiteSetup() {}
   virtual void suiteTearDown() {}
   virtual void runTests() = 0;


public:
   TestSuite(const std::string& suiteName) :
      m_suiteName(suiteName),
      m_numTestsExecuted(0),
      m_numFailures(0) {
   }

   TestSuite(const TestSuite& copy) :
      m_suiteName(copy.m_suiteName),
      m_numTestsExecuted(copy.m_numTestsExecuted),
      m_numFailures(copy.m_numFailures) {
   }

   virtual ~TestSuite() {}

   TestSuite& operator=(const TestSuite& copy) {
      if (this == &copy) {
         return *this;
      }

      m_suiteName = copy.m_suiteName;
      m_numTestsExecuted = copy.m_numTestsExecuted;
      m_numFailures = copy.m_numFailures;

      return *this;
   }

   // file management
   std::string getTempFile() const {
      return "testsuite.txt";
      /*
      char fileTemplate[] = "/tmp/fileXXXXXX.test";
      const int fd = ::mkstemp(fileTemplate);
      if (-1 != fd) {
         return std::string(fileTemplate);
      } else {
         printf("error: mkstemp failed. errno=%d\n", errno);
         return std::string("");
      }
      */
   }

   bool deleteFile(const std::string& filePath) {
      const int rc = remove(filePath.c_str());
      return (0 == rc);
   }

   void run() {
      printf("+++++++++++++ running suite: %s +++++++++++\n", m_suiteName.c_str());
      suiteSetup();
      runTests();
      suiteTearDown();
   }

   const std::string& getName() const {
      return m_suiteName;
   }

   void require(bool expression, std::string testDesc="") {
      if (!expression) {
         ++m_numFailures;
         printf("*** failure: expected true, got false (%s)\n",
                testDesc.c_str());
      }

      ++m_numTestsExecuted;
   }

   void requireFalse(bool expression, std::string testDesc="") {
      if (expression) {
         ++m_numFailures;
         printf("*** failure: expected false, got true (%s)\n",
                testDesc.c_str());
      }

      ++m_numTestsExecuted;
   }

   void requireStringEquals(const std::string& expected,
                            const std::string& actual,
                            std::string testDesc="") {
      if (expected != actual) {
         ++m_numFailures;
         printf("*** failure: expected '%s', actual='%s' (%s)\n",
                expected.c_str(),
                actual.c_str(),
                testDesc.c_str());
      }

      ++m_numTestsExecuted;
   }

   void requireNonEmptyString(const std::string& actual,
                              std::string testDesc="") {
      if (actual.empty()) {
         ++m_numFailures;
         printf("*** failure: expected non-empty string (%s)\n",
                testDesc.c_str());
      }

      ++m_numTestsExecuted;
   }

   void requireException(const char* exceptionType,
                         Runnable* runnable,
                         std::string testDesc="") {
      ++m_numTestsExecuted;
      AutoPointer<Runnable*> uniqueRunnable(runnable);
      try {
         uniqueRunnable->run();
         ++m_numFailures;
         throw BasicException("no exception thrown");
      } catch (BasicException& be) {
         if (strcmp(exceptionType, be.getType())) {
            ++m_numFailures;
            std::string msg(testDesc);
            msg += " - Wrong exception type (";
            msg += "exp=";
            msg += exceptionType;
            msg += ",act=";
            msg += be.getType();
            msg += ")";
            printf("*** failure: %s\n", msg.c_str());
         } else {
            // we got what's expected
         }
      } catch (std::exception& e) {
         //printf("std::exception caught: %s\n", e.what());

         ++m_numFailures;
         std::string msg(testDesc);
         msg += " - Wrong exception type (";
         msg += "exp=";
         msg += exceptionType;
         msg += ",act=std::exception)";
         printf("*** failure: %s\n", msg.c_str());
      } catch (...) {
         //printf("unknown exception caught\n");

         ++m_numFailures;
         std::string msg(testDesc);
         msg += " - Wrong exception type (";
         msg += "exp=";
         msg += exceptionType;
         msg += ",act=unknown)";
         printf("*** failure: %s\n", msg.c_str());
      }
   }

   void startingTestCase(const AbstractTestCase& testCase) {
      printf("--------------- running test: %s -----------\n", testCase.getName().c_str());
      setup();
   }

   void endingTestCase(const AbstractTestCase& testCase) {
      tearDown();
      //printf("ending test case %s\n", testCase.getName().c_str());
   }

   void failTest(const std::string& failureMessage) {
      ++m_numFailures;
      ++m_numTestsExecuted;
      printf("*** failure: %s\n", failureMessage.c_str());
   }
};

}

#endif
