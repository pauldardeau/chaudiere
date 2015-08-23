// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "Tests.h"

#include "TestAutoPointer.h"
#include "TestIniReader.h"
#include "TestInvalidKeyException.h"
#include "TestKeyValuePairs.h"
#include "TestKqueueServer.h"
#include "TestOptionParser.h"
#include "TestPthreadsMutex.h"
#include "TestServerSocket.h"
#include "TestSocket.h"
#include "TestSocketRequest.h"
#include "TestStdMutex.h"
#include "TestStrUtils.h"
#include "TestStringTokenizer.h"
#include "TestThread.h"
#include "TestThreadPool.h"
#include "TestThreadPoolQueue.h"
#include "TestThreadPoolWorker.h"

using namespace chaudiere;

void Tests::run() {
   TestAutoPointer testAutoPointer;
   testAutoPointer.run();
   
   TestIniReader testIniReader;
   testIniReader.run();
   
   TestInvalidKeyException testIKE;
   testIKE.run();
   
   TestKeyValuePairs testKVP;
   testKVP.run();
   
   TestKqueueServer testKqueueServer;
   testKqueueServer.run();
   
   TestOptionParser testOptionParser;
   testOptionParser.run();
   
   TestPthreadsMutex testPthreadsMutex;
   testPthreadsMutex.run();

   TestStdMutex testStdMutex;
   testStdMutex.run();

   TestServerSocket testServerSocket;
   testServerSocket.run();
   
   TestSocket testSocket;
   testSocket.run();
   
   TestSocketRequest testSocketRequest;
   testSocketRequest.run();
   
   TestStringTokenizer testStringTokenizer;
   testStringTokenizer.run();
   
   TestStrUtils testStrUtils;
   testStrUtils.run();
   
   TestThread testThread;
   testThread.run();
   
   TestThreadPool testThreadPool;
   testThreadPool.run();

   TestThreadPoolQueue testThreadPoolQueue;
   testThreadPoolQueue.run();

   TestThreadPoolWorker testThreadPoolWorker;
   testThreadPoolWorker.run();
}

int main(int argc, char* argv[]) {
   Tests tests;
   tests.run();
}
