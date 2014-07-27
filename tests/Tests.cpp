// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "Tests.h"

#include "TestIniReader.h"
#include "TestInvalidKeyException.h"
#include "TestKeyValuePairs.h"
#include "TestKqueueServer.h"
#include "TestPthreadsMutex.h"
#include "TestStdMutex.h"
#include "TestServerSocket.h"
#include "TestSocket.h"
#include "TestSocketRequest.h"
#include "TestStringTokenizer.h"
#include "TestStrUtils.h"
#include "TestThread.h"
#include "TestThreadPool.h"
#include "TestThreadPoolQueue.h"
#include "TestThreadPoolWorker.h"

using namespace chaudiere;

void Tests::run()
{
   TestIniReader testIniReader;
   testIniReader.run();
   
   TestInvalidKeyException testIKE;
   testIKE.run();
   
   TestKeyValuePairs testKVP;
   testKVP.run();
   
   TestKqueueServer testKqueueServer;
   testKqueueServer.run();
   
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

int main(int argc, char* argv[])
{
   Tests tests;
   tests.run();
}
