// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "ThreadingFactory.h"
#include "PthreadsThreadingFactory.h"

using namespace chaudiere;

ThreadingFactory* ThreadingFactory::threadingFactoryInstance = new PthreadsThreadingFactory;

//******************************************************************************

void ThreadingFactory::setThreadingFactory(ThreadingFactory* threadingFactory) {
   threadingFactoryInstance = threadingFactory;
}

//******************************************************************************

ThreadingFactory* ThreadingFactory::getThreadingFactory() {
   return threadingFactoryInstance;
}

//******************************************************************************

