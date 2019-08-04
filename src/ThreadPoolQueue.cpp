// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <cstdio>
#include <cstdlib>

#include "ThreadPoolQueue.h"
#include "ThreadingFactory.h"
#include "ConditionVariable.h"
#include "MutexLock.h"
#include "Logger.h"
#include "BasicException.h"

using namespace chaudiere;

//******************************************************************************

ThreadPoolQueue::ThreadPoolQueue(ThreadingFactory* threadingFactory) :
   m_threadingFactory(threadingFactory),
   m_mutex(nullptr),
   m_condQueueNotEmpty(nullptr),
   m_condQueueNotFull(nullptr),
   m_condQueueEmpty(nullptr),
   m_isInitialized(false),
   m_isRunning(false) {

   LOG_INSTANCE_CREATE("ThreadPoolQueue")

   try {
      m_mutex = std::unique_ptr<Mutex>(m_threadingFactory->createMutex("ThreadPoolQueue"));
      m_condQueueNotEmpty = std::unique_ptr<ConditionVariable>(m_threadingFactory->createConditionVariable("queue-not-empty"));
      m_condQueueNotFull = std::unique_ptr<ConditionVariable>(m_threadingFactory->createConditionVariable("queue-not-full"));
      m_condQueueEmpty = std::unique_ptr<ConditionVariable>(m_threadingFactory->createConditionVariable("queue-empty"));

      m_isInitialized = true;
      m_isRunning = true;
        
      if ((m_mutex != nullptr) &&
          (m_condQueueNotEmpty != nullptr) &&
          (m_condQueueNotFull != nullptr) &&
          (m_condQueueEmpty != nullptr)) {
         m_isInitialized = true;
         m_isRunning = true;
      } else {
         LOG_ERROR("unable to initialize ThreadPoolQueue")
         if (nullptr == m_mutex) {
            LOG_ERROR("unable to create mutex")
         }
         if (nullptr == m_condQueueNotEmpty) {
            LOG_ERROR("unable to create queue not empty condition variable")
         }
         if (nullptr == m_condQueueNotFull) {
            LOG_ERROR("unable to create queue not full condition variable")
         }
         if (nullptr == m_condQueueEmpty) {
            LOG_ERROR("unable to create queue empty condition variable")
         }
         printf("error: unable to initialize thread pool queue, aborting\n");
         exit(1);
      }
   } catch (const BasicException& be) {
      LOG_ERROR("exception setting up thread pool queue: " + be.whatString())
   } catch (const std::exception& e) {
      LOG_ERROR("exception setting up thread pool queue: " + std::string(e.what()))
   } catch (...) {
      LOG_ERROR("unknown exception setting up thread pool queue")
   }
}

//******************************************************************************

ThreadPoolQueue::~ThreadPoolQueue() {
   LOG_INSTANCE_DESTROY("ThreadPoolQueue")
   m_isRunning = false;
}

//******************************************************************************

bool ThreadPoolQueue::addRequest(Runnable* runnableRequest) {
   if (!m_isInitialized) {
      LOG_WARNING("ThreadPoolQueue::addRequest queue not initialized")
      return false;
   }

   if (!runnableRequest) {
      LOG_WARNING("ThreadPoolQueue::addRequest rejecting nullptr request")
      return false;
   }
 
   MutexLock lock(*m_mutex, "ThreadPoolQueue::addRequest");
   
   if (!m_isRunning) {
      LOG_WARNING("ThreadPoolQueue::addRequest rejecting request, queue is shutting down")
      return false;
   }
   
   //if (!m_mutex->haveValidMutex()) {
   //   LOG_ERROR("don't have valid mutex in addRequest")
   //   ::exit(1);
   //}

   LOG_DEBUG("ThreadPoolQueue::addRequest accepting request")
   
   const bool wasEmpty = m_queue.empty();
  
   // add new request to the queue
   m_queue.push_back(runnableRequest);
   
   // did we just transition from QUEUE_EMPTY to QUEUE_NOT_EMPTY?
   if (wasEmpty) {
      // signal QUEUE_NOT_EMPTY (wake up a worker thread)
      LOG_DEBUG("signalling queue_not_empty")
      m_condQueueNotEmpty->notifyAll();
   }
 
   return true;
}

//******************************************************************************

Runnable* ThreadPoolQueue::takeRequest() {
   if (!m_isInitialized) {
      LOG_WARNING("ThreadPoolQueue::takeRequest queue not initialized")
      return nullptr;
   }

   MutexLock lock(*m_mutex, "ThreadPoolQueue::takeRequest");

   // is the queue shut down?
   if (!m_isRunning) {
      return nullptr;
   }
   
   //if (!m_mutex->haveValidMutex()) {
   //   LOG_ERROR("don't have valid mutex in takeRequest")
   //   exit(1);
   //}
   
   Runnable* request = nullptr;
  
   // is the queue empty?
   while (m_queue.empty()) { // && m_isRunning) {
      // empty queue -- wait for QUEUE_NOT_EMPTY event
      m_condQueueNotEmpty->wait(m_mutex.get());
   }

   if (!m_queue.empty()) {
      // take a request from the queue
      request = m_queue.front();
      m_queue.pop_front();

      // did we just empty the queue?
      if (m_queue.empty()) {
         m_condQueueEmpty->notifyOne();
      }
   }

   return request;
}

//******************************************************************************

bool ThreadPoolQueue::shutDown() {
   bool wasShutDown = false;
   if (m_isInitialized && m_isRunning) {
      MutexLock lock(*m_mutex, "ThreadPoolQueue::shutDown");
      m_isRunning = false;
      wasShutDown = true;
   }
   return wasShutDown;
}

//******************************************************************************

bool ThreadPoolQueue::isRunning() const {
   return m_isRunning;
}

//******************************************************************************

bool ThreadPoolQueue::isEmpty() const {
   return m_queue.empty();
}

//******************************************************************************

bool ThreadPoolQueue::isInitialized() const {
   return m_isInitialized;
}

//******************************************************************************

