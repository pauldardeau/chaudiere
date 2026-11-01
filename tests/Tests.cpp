// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestAutoPointer.h"
#include "TestByteBuffer.h"
#include "TestCharBuffer.h"
#include "TestDateTime.h"
#include "TestDynamicLibrary.h"
#include "TestEpollServer.h"
#include "TestFileLogger.h"
#include "TestIniReader.h"
#include "TestInvalidKeyException.h"
#include "TestKeyValuePairs.h"
#include "TestKqueueServer.h"
#include "TestMutexLock.h"
#include "TestNumberFormatException.h"
#include "TestOptionParser.h"
#include "TestOSUtils.h"
#include "TestPthreadsConditionVariable.h"
#include "TestPthreadsMutex.h"
#include "TestPthreadsThreadingFactory.h"
#include "TestRequestHandler.h"
#include "TestServerSocket.h"
#include "TestServiceInfo.h"
#include "TestSocket.h"
#include "TestSocketRequest.h"
#include "TestSocketServer.h"
#include "TestStdConditionVariable.h"
#include "TestStdLogger.h"
#include "TestStdMutex.h"
#include "TestStdThread.h"
#include "TestStdThreadingFactory.h"
#include "TestStrUtils.h"
#include "TestStringTokenizer.h"
#include "TestSystemInfo.h"
#include "TestSystemStats.h"
#include "TestThread.h"
#include "TestThreadInfo.h"
#include "TestThreadPoolQueue.h"
#include "TestThreadPoolWorker.h"
#include "TestThreadingFactory.h"
#include "TestUtils.h"

#include "TestRegistry.h"

#include <stdio.h>
#include <string>
#include <vector>

using namespace chaudiere;

namespace {

// totals over every hand-listed (classic) suite run by run_tests(). The
// POIVRE_TEST_CASE cases are counted separately by the TestRegistry, so
// nothing is added to both.
struct ClassicTotals {
   int suitesRun = 0;
   int testCasesRun = 0;
   int assertions = 0;
   int failures = 0;
   std::vector<std::string> failedSuites;
};

ClassicTotals g_classicTotals;

}

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
   run_test(new TestDynamicLibrary);
   run_test(new TestEpollServer);
   run_test(new TestFileLogger);
   run_test(new TestIniReader);
   run_test(new TestInvalidKeyException);
   run_test(new TestKeyValuePairs);
   run_test(new TestKqueueServer);
   run_test(new TestMutexLock);
   run_test(new TestNumberFormatException);
   run_test(new TestOptionParser);
   run_test(new TestOSUtils);
   run_test(new TestPthreadsConditionVariable);
   run_test(new TestPthreadsMutex);
   run_test(new TestPthreadsThreadingFactory);
   run_test(new TestStdMutex);
   run_test(new TestRequestHandler);
   run_test(new TestServerSocket);
   run_test(new TestServiceInfo);
   run_test(new TestSocket);
   run_test(new TestSocketRequest);
   run_test(new TestSocketServer);
   run_test(new TestStdConditionVariable);
   run_test(new TestStdLogger);
   run_test(new TestStdThread);
   run_test(new TestStdThreadingFactory);
   run_test(new TestStringTokenizer);
   run_test(new TestStrUtils);
   run_test(new TestSystemInfo);
   run_test(new TestSystemStats);
   run_test(new TestThread);
   run_test(new TestThreadInfo);
   run_test(new TestThreadPoolQueue);
   run_test(new TestThreadPoolWorker);
   run_test(new TestThreadingFactory);
   run_test(new TestUtils);
}

int main(int argc, char* argv[]) {
   run_tests();

   // runs every POIVRE_TEST_CASE self-registered test case (e.g.
   // TestThreadPool's - see its .cpp) that isn't hand-listed above in
   // run_tests(). Prints its own summary (registered cases only).
   poivre::TestRegistry& registry = poivre::TestRegistry::instance();
   registry.runAll();

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

   // 0 only if at least one assertion ran and nothing failed. Never the raw
   // failure count: exit statuses are truncated (256 failures would be 0).
   return poivre::TestRegistry::exitCodeFor(testCases, assertions, failures);
}
