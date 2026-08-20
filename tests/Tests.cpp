// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestAutoPointer.h"
#include "TestByteBuffer.h"
#include "TestDateTime.h"
#include "TestDynamicLibrary.h"
#include "TestFileLogger.h"
#include "TestIniReader.h"
#include "TestInvalidKeyException.h"
#include "TestKeyValuePairs.h"
#include "TestKqueueServer.h"
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
#include "TestStrUtils.h"
#include "TestStringTokenizer.h"
#include "TestSystemInfo.h"
#include "TestSystemStats.h"
#include "TestThread.h"
#include "TestThreadInfo.h"
#include "TestThreadPool.h"
#include "TestThreadPoolQueue.h"
#include "TestThreadPoolWorker.h"
#include "TestThreadingFactory.h"
#include "TestUtils.h"

using namespace chaudiere;

void run_test(poivre::TestSuite* test_suite) {
   if (nullptr != test_suite) {
      test_suite->run();
      delete test_suite;
   }
}

void run_tests() {
   run_test(new TestAutoPointer);
   run_test(new TestByteBuffer);
   run_test(new TestDateTime);
   run_test(new TestDynamicLibrary);
   run_test(new TestFileLogger);
   run_test(new TestIniReader);
   run_test(new TestInvalidKeyException);
   run_test(new TestKeyValuePairs);
   run_test(new TestKqueueServer);
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
   run_test(new TestStringTokenizer);
   run_test(new TestStrUtils);
   run_test(new TestSystemInfo);
   run_test(new TestSystemStats);
   run_test(new TestThread);
   run_test(new TestThreadInfo);
   run_test(new TestThreadPool);
   run_test(new TestThreadPoolQueue);
   run_test(new TestThreadPoolWorker);
   run_test(new TestThreadingFactory);
   run_test(new TestUtils);
}

int main(int argc, char* argv[]) {
   run_tests();
}
