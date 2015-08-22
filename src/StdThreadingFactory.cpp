// Copyright Paul Dardeau, SwampBits LLC 2015
// BSD License

#include "StdThreadingFactory.h"
#include "StdMutex.h"
#include "StdThread.h"
#include "ThreadPool.h"
#include "Logger.h"
#include "StdConditionVariable.h"

using namespace chaudiere;

//******************************************************************************

StdThreadingFactory::StdThreadingFactory() noexcept {
   Logger::logInstanceCreate("StdThreadingFactory");
}

//******************************************************************************

StdThreadingFactory::~StdThreadingFactory() noexcept {
   Logger::logInstanceDestroy("StdThreadingFactory");
}

//******************************************************************************

Mutex* StdThreadingFactory::createMutex() {
   return new StdMutex();
}

//******************************************************************************

Mutex* StdThreadingFactory::createMutex(const std::string& mutexName) {
   return new StdMutex(mutexName);
}

//******************************************************************************

Thread* StdThreadingFactory::createThread() noexcept {
   return createThread(nullptr);
}

//******************************************************************************

Thread* StdThreadingFactory::createThread(Runnable* runnable) noexcept {
   if (runnable != nullptr) {
      return new StdThread(runnable);
   } else {
      return new StdThread();
   }
}

//******************************************************************************

ConditionVariable* StdThreadingFactory::createConditionVariable() {
   return new StdConditionVariable();
}

//******************************************************************************

ThreadPoolDispatcher* StdThreadingFactory::createThreadPoolDispatcher(int numberThreads) noexcept {
   return new ThreadPool(this, numberThreads);
}

//******************************************************************************

