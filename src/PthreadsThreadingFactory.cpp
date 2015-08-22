// Copyright Paul Dardeau, SwampBits LLC 2015
// BSD License

#include "PthreadsThreadingFactory.h"
#include "PthreadsMutex.h"
#include "PthreadsThread.h"
#include "ThreadPool.h"
#include "Logger.h"
#include "PthreadsConditionVariable.h"

using namespace chaudiere;

//******************************************************************************

PthreadsThreadingFactory::PthreadsThreadingFactory() noexcept {
   Logger::logInstanceCreate("PthreadsThreadingFactory");
}

//******************************************************************************

PthreadsThreadingFactory::~PthreadsThreadingFactory() noexcept {
   Logger::logInstanceDestroy("PthreadsThreadingFactory");
}

//******************************************************************************

Mutex* PthreadsThreadingFactory::createMutex() {
   return new PthreadsMutex();
}

//******************************************************************************

Mutex* PthreadsThreadingFactory::createMutex(const std::string& mutexName) {
   return new PthreadsMutex(mutexName);
}

//******************************************************************************

Thread* PthreadsThreadingFactory::createThread() noexcept {
   return createThread(nullptr);
}

//******************************************************************************

Thread* PthreadsThreadingFactory::createThread(Runnable* runnable) noexcept {
   return new PthreadsThread(runnable);
}

//******************************************************************************

ConditionVariable* PthreadsThreadingFactory::createConditionVariable() {
   return new PthreadsConditionVariable();
}

//******************************************************************************

ThreadPoolDispatcher* PthreadsThreadingFactory::createThreadPoolDispatcher(int numberThreads) noexcept {
   return new ThreadPool(this, numberThreads);
}

//******************************************************************************

