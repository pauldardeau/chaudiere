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

StdThreadingFactory::StdThreadingFactory() {
   Logger::logInstanceCreate("StdThreadingFactory");
}

//******************************************************************************

StdThreadingFactory::~StdThreadingFactory() {
   Logger::logInstanceDestroy("StdThreadingFactory");
}

//******************************************************************************

Mutex* StdThreadingFactory::createMutex(const std::string& name) {
   return new StdMutex(name);
}

//******************************************************************************

Thread* StdThreadingFactory::createThread(const std::string& name) {
   return createThread(NULL, name);
}

//******************************************************************************

Thread* StdThreadingFactory::createThread(Runnable* runnable, const std::string& name) {
   if (runnable != NULL) {
      return new StdThread(runnable, name);
   } else {
      return new StdThread(name);
   }
}

//******************************************************************************

ConditionVariable* StdThreadingFactory::createConditionVariable(const std::string& name) {
   return new StdConditionVariable(name);
}

//******************************************************************************

ThreadPoolDispatcher* StdThreadingFactory::createThreadPoolDispatcher(int numberThreads, const std::string& name) {
   return new ThreadPool(this, numberThreads, name);
}

//******************************************************************************

