// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <cstdio>

#include "ThreadPool.h"
#include "ThreadPoolQueue.h"
#include "Logger.h"

using namespace std;
using namespace chaudiere;

//******************************************************************************

ThreadPool::ThreadPool(int numberWorkers) :
   m_threadingFactory(ThreadingFactory::getThreadingFactory()),
   m_queue(m_threadingFactory),
   m_workerCount(numberWorkers),
   m_workersCreated(0),
   m_isRunning(false),
   m_name("") {
}

//******************************************************************************

ThreadPool::ThreadPool(int numberWorkers, const std::string& name) :
   m_threadingFactory(ThreadingFactory::getThreadingFactory()),
   m_queue(m_threadingFactory),
   m_workerCount(numberWorkers),
   m_workersCreated(0),
   m_isRunning(false),
   m_name(name) {
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
}

//******************************************************************************

ThreadPool::~ThreadPool() {
   Logger::logInstanceDestroy("ThreadPool");

   if (m_isRunning) {
      stop();
   }
   
   m_listWorkers.clear();
}

//******************************************************************************

bool ThreadPool::start() {
   for (int i = 0; i < m_workerCount; ++i) {
      ++m_workersCreated;
      unique_ptr<ThreadPoolWorker> worker(
         new ThreadPoolWorker(m_threadingFactory, m_queue, m_workersCreated));
      worker->start();
      m_listWorkers.push_back(std::move(worker));
   }

   m_isRunning = true;
   return true;
}

//******************************************************************************

bool ThreadPool::stop() {
   m_queue.shutDown();
 
   for (unique_ptr<ThreadPoolWorker>& w : m_listWorkers) {
      w->stop();
   }

   m_isRunning = false;
   return true;
}

//******************************************************************************

bool ThreadPool::addRequest(Runnable* runnableRequest) {
   if (!m_isRunning) {
      return false;
   }
   
   m_queue.addRequest(runnableRequest);
   
   return true;
}

//******************************************************************************

unique_ptr<Thread> ThreadPool::createThreadWithRunnable(Runnable* runnable) {
   return m_threadingFactory->createThread(runnable, "threadpool");
}

//******************************************************************************

int ThreadPool::getNumberWorkers() const {
   return m_workerCount;
}

//******************************************************************************

void ThreadPool::addWorkers(int numberNewWorkers) {
   adjustNumberWorkers(numberNewWorkers);
}

//******************************************************************************

void ThreadPool::removeWorkers(int numberWorkersToRemove) {
   adjustNumberWorkers(-numberWorkersToRemove);
}

//******************************************************************************

void ThreadPool::adjustNumberWorkers(int numberToAddOrDelete) {
   if (numberToAddOrDelete > 0) {   // adding?

      const int newNumberWorkers = m_workerCount + numberToAddOrDelete;

      for (int i = m_workerCount; i < newNumberWorkers; ++i) {
         ++m_workersCreated;
         ++m_workerCount;
         unique_ptr<ThreadPoolWorker> worker(
            new ThreadPoolWorker(m_threadingFactory, m_queue, m_workersCreated));

         if (m_isRunning) {
            worker->start();
         }

         m_listWorkers.push_back(std::move(worker));
      }
   } else if (numberToAddOrDelete < 0) {  // removing?
      if (m_isRunning) {
         //TODO: add support for terminating some threads
      }
   }
}

//******************************************************************************

const std::string& ThreadPool::getName() const {
   return m_name;
}

