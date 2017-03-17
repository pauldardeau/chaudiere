// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "ThreadingFactory.h"

using namespace chaudiere;

ThreadingFactory* ThreadingFactory::threadingFactoryInstance = NULL;

//******************************************************************************

void ThreadingFactory::setThreadingFactory(ThreadingFactory* threadingFactory) {
   threadingFactoryInstance = threadingFactory;
}

//******************************************************************************

ThreadingFactory* ThreadingFactory::getThreadingFactory() {
   return threadingFactoryInstance;
}

//******************************************************************************

