// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "ThreadingFactory.h"

using namespace std;
using namespace chaudiere;

shared_ptr<ThreadingFactory> ThreadingFactory::threadingFactoryInstance(nullptr);

//******************************************************************************

void ThreadingFactory::setThreadingFactory(shared_ptr<ThreadingFactory>& threadingFactory) {
   threadingFactoryInstance = threadingFactory;
}

//******************************************************************************

shared_ptr<ThreadingFactory>& ThreadingFactory::getThreadingFactory() {
   return threadingFactoryInstance;
}

//******************************************************************************

