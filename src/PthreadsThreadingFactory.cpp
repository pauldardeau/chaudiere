// Copyright Paul Dardeau, SwampBits LLC 2015
// BSD License

#include "PthreadsThreadingFactory.h"
#include "PthreadsMutex.h"
#include "PthreadsThread.h"
#include "ThreadPool.h"
#include "Logger.h"
#include "PthreadsConditionVariable.h"

using namespace std;
using namespace chaudiere;

//******************************************************************************

PthreadsThreadingFactory::PthreadsThreadingFactory() {
   Logger::logInstanceCreate("PthreadsThreadingFactory");
}

//******************************************************************************

PthreadsThreadingFactory::~PthreadsThreadingFactory() {
   Logger::logInstanceDestroy("PthreadsThreadingFactory");
}

//******************************************************************************

unique_ptr<Mutex> PthreadsThreadingFactory::createMutex(const std::string& name) {
   return unique_ptr<Mutex>(new PthreadsMutex(name));
}

//******************************************************************************

unique_ptr<Thread> PthreadsThreadingFactory::createThread(const std::string& name) {
   return createThread(NULL, name);
}

//******************************************************************************

unique_ptr<Thread> PthreadsThreadingFactory::createThread(Runnable* runnable,
           const std::string& name) {
   return unique_ptr<Thread>(new PthreadsThread(runnable, name));
}

//******************************************************************************

unique_ptr<ConditionVariable> PthreadsThreadingFactory::createConditionVariable(const std::string& name) {
   return unique_ptr<ConditionVariable>(new PthreadsConditionVariable(name));
}

//******************************************************************************

unique_ptr<ThreadPoolDispatcher> PthreadsThreadingFactory::createThreadPoolDispatcher(int numberThreads,
                                                                           const std::string& name) {
   return unique_ptr<ThreadPoolDispatcher>(new ThreadPool(this, numberThreads, name));
}


