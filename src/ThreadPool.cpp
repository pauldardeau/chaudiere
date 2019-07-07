// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <cstdio>

#include "ThreadPool.h"
#include "ThreadPoolQueue.h"
#include "Logger.h"

using namespace chaudiere;

//******************************************************************************

ThreadPool::ThreadPool(int numberWorkers) :
   m_threadingFactory(ThreadingFactory::getThreadingFactory()),
   m_queue(m_threadingFactory),
   m_workerCount(numberWorkers),
   m_workersCreated(0),
   m_isRunning(false),
   m_name("") {
   start();
}

//******************************************************************************

ThreadPool::ThreadPool(int numberWorkers, const std::string& name) :
   m_threadingFactory(ThreadingFactory::getThreadingFactory()),
   m_queue(m_threadingFactory),
   m_workerCount(numberWorkers),
   m_workersCreated(0),
   m_isRunning(false),
   m_name(name) {
   start();
}

//*****************************************************************************

ThreadPool::ThreadPool(ThreadingFactory* threadingFactory,
                       int numberWorkers) :
   m_threadingFactory(threadingFactory),
   m_queue(m_threadingFactory),
   m_workerCount(numberWorkers),
   m_workersCreated(0),
   m_isRunning(false),
   m_name("") {
   start();
}

//*****************************************************************************

ThreadPool::ThreadPool(ThreadingFactory* threadingFactory,
                       int numberWorkers,
                       const std::string& name) :
   m_threadingFactory(threadingFactory),
   m_queue(m_threadingFactory),
   m_workerCount(numberWorkers),
   m_workersCreated(0),
   m_isRunning(false),
   m_name(name) {
   Logger::logInstanceCreate("ThreadPool");
   start();
}

//******************************************************************************

ThreadPool::~ThreadPool() {
   Logger::logInstanceDestroy("ThreadPool");

   if (m_isRunning) {
      stop();
   }
   
   std::list<ThreadPoolWorker*>::iterator it;
   for (it = m_listWorkers.begin(); it != m_listWorkers.end(); ++it) {
       delete *it;
   }
   
   m_listWorkers.clear();
}

//******************************************************************************

bool ThreadPool::start() {
   //printf("ThreadPool::start entered\n");
   bool didStart = false;
   int oldCreatedCount = m_workersCreated;
   //printf("ThreadPool::start oldCreatedCount=%d\n", oldCreatedCount);

   if (!m_isRunning) {
      for (int i = 0; i < m_workerCount; ++i) {
         ++m_workersCreated;
         ThreadPoolWorker* worker =
            new ThreadPoolWorker(m_threadingFactory, m_queue, m_workersCreated);
         worker->start();
         m_listWorkers.push_back(worker);
      }

      int newCreatedCount = m_workersCreated - oldCreatedCount;
      //printf("ThreadPool::start newCreatedCount = %d\n", newCreatedCount);

      if (newCreatedCount > 0) {
         //printf("ThreadPool::start positive newCreatedCount\n");
         m_isRunning = true;
         didStart = true;
      } else {
         printf("ThreadPool::start created count is not positive, did not start\n");
      }
   }

   return didStart;
}

//******************************************************************************

bool ThreadPool::stop() {
   bool didStop = false;

   if (m_isRunning) {
      m_queue.shutDown();
  
      std::list<ThreadPoolWorker*>::iterator it = m_listWorkers.begin();
      const std::list<ThreadPoolWorker*>::const_iterator itEnd =
         m_listWorkers.end();
 
      for (; it != itEnd; it++) {
         (*it)->stop();
      }
   
      m_isRunning = false;
      didStop = true;
   }

   return didStop;
}

//******************************************************************************

bool ThreadPool::addRequest(Runnable* runnableRequest) {
   //printf("ThreadPool::addRequest entered\n");
   bool requestAdded = false;

   if (m_isRunning && (NULL != runnableRequest)) {
      //printf("ThreadPool::addRequest adding request to queue\n");
      m_queue.addRequest(runnableRequest);
      //printf("ThreadPool::addRequest back from adding to queue\n");
      requestAdded = true;
   } else {
      if (!m_isRunning) {
         printf("ThreadPool::addRequest m_isRunning is false\n");
      }
      if (NULL == runnableRequest) {
         printf("ThreadPool::addRequest request is NULL\n");
      }
   }
   
   return requestAdded;
}

//******************************************************************************

Thread* ThreadPool::createThreadWithRunnable(Runnable* runnable) {
   return m_threadingFactory->createThread(runnable, "threadpool");
}

//******************************************************************************

int ThreadPool::getNumberWorkers() const {
   return m_workerCount;
}

//******************************************************************************

bool ThreadPool::addWorkers(int numberNewWorkers) {
   if (numberNewWorkers > 0) {
      return adjustNumberWorkers(numberNewWorkers);
   } else {
      return false;
   }
}

//******************************************************************************

bool ThreadPool::removeWorkers(int numberWorkersToRemove) {
   if (numberWorkersToRemove > 0) {
      return adjustNumberWorkers(-numberWorkersToRemove);
   } else {
      return false;
   }
}

//******************************************************************************

bool ThreadPool::adjustNumberWorkers(int numberToAddOrDelete) {
   bool requestSatisfied = false;

   if (numberToAddOrDelete > 0) {   // adding?

      const int newNumberWorkers = m_workerCount + numberToAddOrDelete;

      for (int i = m_workerCount; i < newNumberWorkers; ++i) {
         ++m_workersCreated;
         ++m_workerCount;
         ThreadPoolWorker* worker =
            new ThreadPoolWorker(m_threadingFactory, m_queue, m_workersCreated);

         if (m_isRunning) {
            worker->start();
         }

         m_listWorkers.push_back(worker);
      }
      requestSatisfied = true;
   } else if (numberToAddOrDelete < 0) {  // removing?
      if (m_isRunning) {
         int numToRemove = -numberToAddOrDelete;
         if (numToRemove <= m_workerCount) {
            while (numToRemove > 0) {
               ThreadPoolWorker* lastWorker = m_listWorkers.back();
               m_listWorkers.pop_back();
               --m_workerCount;
               --numToRemove;
               lastWorker->stop();
               delete lastWorker;
            }
            requestSatisfied = true;
         } 
      }
   } else {
      // caller asked for no adjustment and we satisfied request
      requestSatisfied = true; 
   }

   return requestSatisfied;
}

//******************************************************************************

const std::string& ThreadPool::getName() const {
   return m_name;
}

//******************************************************************************

bool ThreadPool::isRunning() const {
   return m_isRunning;
}

//******************************************************************************

