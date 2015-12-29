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

ThreadPoolQueue::ThreadPoolQueue(ThreadingFactory* threadingFactory) noexcept :
   m_threadingFactory(threadingFactory),
   m_mutex(NULL),
   m_condQueueEmpty(NULL),
   m_condQueueNotEmpty(NULL),
   m_isInitialized(false),
   m_isRunning(false) {

   Logger::logInstanceCreate("ThreadPoolQueue");

   try {
      m_mutex = m_threadingFactory->createMutex("ThreadPoolQueue");
      m_condQueueEmpty = m_threadingFactory->createConditionVariable("queue-empty");
      m_condQueueNotEmpty = m_threadingFactory->createConditionVariable("queue-not-empty");
         
      if ((m_mutex != NULL) &&
          (m_condQueueEmpty != NULL) &&
          (m_condQueueNotEmpty != NULL)) {
         m_isInitialized = true;
         m_isRunning = true;
      } else {
         Logger::error("unable to initialize ThreadPoolQueue");
         if (NULL == m_mutex) {
            Logger::error("unable to create mutex");
         }
         if (NULL == m_condQueueEmpty) {
            Logger::error("unable to create queue empty condition variable");
         }
         if (NULL == m_condQueueNotEmpty) {
            Logger::error("unable to create queue not empty condition variable");
         }
         printf("error: unable to initialize thread pool queue, aborting\n");
         exit(1);
      }
   } catch (const BasicException& be) {
      Logger::error("exception setting up thread pool queue: " + be.whatString());
   } catch (const std::exception& e) {
      Logger::error("exception setting up thread pool queue: " + std::string(e.what()));
   } catch (...) {
      Logger::error("unknown exception setting up thread pool queue");
   }
}

//******************************************************************************

ThreadPoolQueue::~ThreadPoolQueue() noexcept {
   Logger::logInstanceDestroy("ThreadPoolQueue");
   m_isRunning = false;
   if (NULL != m_mutex) {
      delete m_mutex;
   }
   if (NULL != m_condQueueEmpty) {
      delete m_condQueueEmpty;
   }
   if (NULL != m_condQueueNotEmpty) {
      delete m_condQueueNotEmpty;
   }
}

//******************************************************************************

bool ThreadPoolQueue::addRequest(Runnable* runnableRequest) noexcept {
   if (!m_isInitialized) {
      Logger::log(Logger::LogLevel::Warning, "ThreadPoolQueue::addRequest queue not initialized");
      return false;
   }

   if (!runnableRequest) {
      Logger::log(Logger::LogLevel::Warning, "ThreadPoolQueue::addRequest rejecting NULL request");
      return false;
   }
   
   MutexLock lock(*m_mutex);
   
   if (!m_isRunning) {
      Logger::log(Logger::LogLevel::Warning, "ThreadPoolQueue::addRequest rejecting request, queue is shutting down");
      return false;
   }
   
   if (!m_mutex->haveValidMutex()) {
      Logger::error("don't have valid mutex in addRequest");
      ::exit(1);
   }

   Logger::log(Logger::LogLevel::Debug, "ThreadPoolQueue::addRequest accepting request");
   
   const bool wasEmpty = m_queue.empty();
   
   // add new request to the queue
   m_queue.push_back(runnableRequest);
   
   // did we just transition from QUEUE_EMPTY to QUEUE_NOT_EMPTY?
   if (wasEmpty && !m_queue.empty()) {
      // signal QUEUE_NOT_EMPTY (wake up a worker thread)
      Logger::log(Logger::LogLevel::Debug, "signalling queue_not_empty");
      m_condQueueNotEmpty->notifyAll();
   }
   
   return true;
}

//******************************************************************************

Runnable* ThreadPoolQueue::takeRequest() noexcept {
   if (!m_isInitialized) {
      Logger::log(Logger::LogLevel::Warning, "ThreadPoolQueue::takeRequest queue not initialized");
      return NULL;
   }

   MutexLock lock(*m_mutex);

   // is the queue shut down?
   if (!m_isRunning) {
      return NULL;
   }
   
   if (!m_mutex->haveValidMutex()) {
      Logger::error("don't have valid mutex in takeRequest");
      exit(1);
   }
   
   Runnable* request = NULL;
  
   // is the queue empty?
   while (m_queue.empty() && m_isRunning) {
      //printf( "queue is empty, waiting for QUEUE_NOT_EMPTY event\n" );

      // empty queue -- wait for QUEUE_NOT_EMPTY event
      m_condQueueNotEmpty->wait(m_mutex);
   }

   if (!m_queue.empty()) {
      // take a request from the queue
      request = m_queue.front();
      m_queue.pop_front();
         
      // did we empty the queue?
      if (m_queue.empty()) {
         Logger::log(Logger::LogLevel::Debug, "--- signalling queue_empty_event");
         // signal that queue is now empty
         m_condQueueEmpty->notifyOne();
      }
   }

   return request;
}

//******************************************************************************

void ThreadPoolQueue::shutDown() noexcept {
   printf("ThreadPoolQueue::shutDown\n");
   if (m_isInitialized && m_isRunning) {
      MutexLock lock(*m_mutex);
      m_isRunning = false;
   
      if (!m_queue.empty()) {
         // wait for QUEUE_EMPTY event
         m_condQueueEmpty->wait(m_mutex);
      }
   }
}

//******************************************************************************

bool ThreadPoolQueue::isRunning() const noexcept {
   return m_isRunning;
}

//******************************************************************************

bool ThreadPoolQueue::isEmpty() const noexcept {
   printf("ThreadPoolQueue::isEmpty\n");
   //MutexLock lock(*m_mutex);
   return m_queue.empty();
}

//******************************************************************************

bool ThreadPoolQueue::isInitialized() const noexcept {
   return m_isInitialized;
}

//******************************************************************************

