// Copyright Paul Dardeau, SwampBits LLC 2015
// BSD License

#include <stdio.h>

#include "PthreadsThreadingFactory.h"
#include "PthreadsMutex.h"
#include "PthreadsThread.h"
#include "ThreadPool.h"
#include "Logger.h"
#include "PthreadsConditionVariable.h"

using namespace chaudiere;

//******************************************************************************

PthreadsThreadingFactory::PthreadsThreadingFactory() {
   LOG_INSTANCE_CREATE("PthreadsThreadingFactory")
}

//******************************************************************************

PthreadsThreadingFactory::~PthreadsThreadingFactory() {
   LOG_INSTANCE_DESTROY("PthreadsThreadingFactory")
}

//******************************************************************************

Mutex* PthreadsThreadingFactory::createMutex(const std::string& name) {
   return new PthreadsMutex(name);
}

//******************************************************************************

Thread* PthreadsThreadingFactory::createThread(const std::string& name) {
   return createThread(nullptr, name);
}

//******************************************************************************

Thread* PthreadsThreadingFactory::createThread(Runnable* runnable,
           const std::string& name) {
   return new PthreadsThread(runnable, name);
}

//******************************************************************************

ConditionVariable* PthreadsThreadingFactory::createConditionVariable(const std::string& name) {
   return new PthreadsConditionVariable(name);
}

//******************************************************************************

ThreadPoolDispatcher* PthreadsThreadingFactory::createThreadPoolDispatcher(int numberThreads,
                                                                           const std::string& name) {
   return new ThreadPool(this, numberThreads, name);
}

//******************************************************************************
