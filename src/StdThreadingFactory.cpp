// Copyright Paul Dardeau, SwampBits LLC 2015
// BSD License

#include "StdThreadingFactory.h"
#include "StdMutex.h"
#include "StdThread.h"
#include "ThreadPool.h"
#include "Logger.h"
#include "StdConditionVariable.h"

using namespace std;
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

unique_ptr<Mutex> StdThreadingFactory::createMutex(const std::string& name) {
   return unique_ptr<Mutex>(new StdMutex(name));
}

//******************************************************************************

unique_ptr<Thread> StdThreadingFactory::createThread(const std::string& name) {
   return createThread(nullptr, name);
}

//******************************************************************************

unique_ptr<Thread> StdThreadingFactory::createThread(Runnable* runnable, const std::string& name) {
   if (runnable != nullptr) {
      return unique_ptr<Thread>(new StdThread(runnable, name));
   } else {
      return unique_ptr<Thread>(new StdThread(name));
   }
}

//******************************************************************************

unique_ptr<ConditionVariable> StdThreadingFactory::createConditionVariable(const std::string& name) {
   return unique_ptr<ConditionVariable>(new StdConditionVariable(name));
}

//******************************************************************************

unique_ptr<ThreadPoolDispatcher> StdThreadingFactory::createThreadPoolDispatcher(int numberThreads, const std::string& name) {
   return unique_ptr<ThreadPoolDispatcher>(new ThreadPool(numberThreads, name));
}

//******************************************************************************

