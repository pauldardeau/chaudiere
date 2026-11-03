// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

// The test driver for the Windows (MSVC) build of test_chaudiere. Tests.cpp
// lists every suite and so needs every POSIX-only component; this lists only
// the suites that can run on Windows, and tests/CMakeLists.txt records the
// ones that are left out and why.

#include "TestAutoPointer.h"
#include "TestByteBuffer.h"
#include "TestCharBuffer.h"
#include "TestDateTime.h"
#include "TestIniReader.h"
#include "TestInvalidKeyException.h"
#include "TestKeyValuePairs.h"
#include "TestNumberFormatException.h"
#include "TestOptionParser.h"
#include "TestOSUtils.h"
#include "TestServiceInfo.h"
#include "TestStrUtils.h"
#include "TestStringTokenizer.h"
#include "TestUtils.h"

#include "TestRegistry.h"

#include <stdio.h>
#include <string>
#include <vector>

using namespace chaudiere;

namespace {

// totals over every hand-listed (classic) suite run by run_tests()
struct ClassicTotals {
   int suitesRun = 0;
   int testCasesRun = 0;
   int assertions = 0;
   int failures = 0;
   std::vector<std::string> failedSuites;
};

ClassicTotals g_classicTotals;

void run_test(poivre::TestSuite* test_suite) {
   if (nullptr != test_suite) {
      test_suite->run();

      ++g_classicTotals.suitesRun;
      g_classicTotals.testCasesRun += test_suite->getNumTestCasesRun();
      g_classicTotals.assertions += test_suite->getNumTestsExecuted();
      g_classicTotals.failures += test_suite->getNumFailures();
      if (test_suite->getNumFailures() > 0) {
         g_classicTotals.failedSuites.push_back(test_suite->getName());
      }

      delete test_suite;
   }
}

void run_tests() {
   run_test(new TestAutoPointer);
   run_test(new TestByteBuffer);
   run_test(new TestCharBuffer);
   run_test(new TestDateTime);
   run_test(new TestIniReader);
   run_test(new TestInvalidKeyException);
   run_test(new TestKeyValuePairs);
   run_test(new TestNumberFormatException);
   run_test(new TestOptionParser);
   run_test(new TestOSUtils);
   run_test(new TestServiceInfo);
   run_test(new TestStringTokenizer);
   run_test(new TestStrUtils);
   run_test(new TestUtils);
}

}

int main() {
   run_tests();

   // any self-registered POIVRE_TEST_CASE cases (none are built for Windows
   // today, but a suite added later may register some). runAll() on an empty
   // registry prints a "no test cases were executed ... reports failure"
   // banner that would only mislead here, where the classic suites above
   // carry the result.
   poivre::TestRegistry& registry = poivre::TestRegistry::instance();
   if (registry.getNumRegisteredTestCases() > 0) {
      registry.runAll();
   }

   const int testCases =
      g_classicTotals.testCasesRun + registry.getNumTestCasesRun();
   const int assertions =
      g_classicTotals.assertions + registry.getNumAssertions();
   const int failures =
      g_classicTotals.failures + registry.getNumFailures();

   printf("\n=============================================\n");
   printf("test_chaudiere overall: %d classic suite(s), %d test case(s), "
          "%d assertion(s) executed, %d failed\n",
          g_classicTotals.suitesRun, testCases, assertions, failures);
   for (const std::string& name : g_classicTotals.failedSuites) {
      printf("*** failed suite: %s\n", name.c_str());
   }
   if (registry.getNumFailures() > 0) {
      printf("*** %d failure(s) in registered POIVRE_TEST_CASE cases\n",
             registry.getNumFailures());
   }
   printf("=============================================\n");
   fflush(stdout);

   // 0 only if at least one assertion ran and nothing failed
   return poivre::TestRegistry::exitCodeFor(testCases, assertions, failures);
}
