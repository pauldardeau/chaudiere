// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "ThreadingFactory.h"

using namespace chaudiere;

ThreadingFactory* ThreadingFactory::threadingFactoryInstance = nullptr;

//******************************************************************************

void ThreadingFactory::setThreadingFactory(ThreadingFactory* threadingFactory) noexcept {
   threadingFactoryInstance = threadingFactory;
}

//******************************************************************************

ThreadingFactory* ThreadingFactory::getThreadingFactory() noexcept {
   return threadingFactoryInstance;
}

//******************************************************************************

