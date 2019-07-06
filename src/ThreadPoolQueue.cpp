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
   //m_mutex(NULL),
   //m_condQueueNotEmpty(NULL),
   //m_condQueueNotFull(NULL),
   //m_condQueueEmpty(NULL),
   m_isInitialized(false),
   m_isRunning(false) {

   Logger::logInstanceCreate("ThreadPoolQueue");

   try {
      /*
      m_mutex = m_threadingFactory->createMutex("ThreadPoolQueue");
      m_condQueueNotEmpty = m_threadingFactory->createConditionVariable("queue-not-empty");
      m_condQueueNotFull = m_threadingFactory->createConditionVariable("queue-not-full");
      m_condQueueEmpty = m_threadingFactory->createConditionVariable("queue-empty");
      */

      int rc;
      rc = pthread_mutex_init(&m_queue_lock, NULL);
      if (rc != 0) {
         printf("error: pthread_mutex_init failed\n");
         exit(1);
      }
      rc = pthread_cond_init(&m_cond_queue_not_empty, NULL);
      if (rc != 0) {
         printf("error: pthread_cond_init failed\n");
         exit(1);
      }
      rc = pthread_cond_init(&m_cond_queue_not_full, NULL);
      if (rc != 0) {
         printf("error: pthread_cond_init failed\n");
         exit(1);
      }
      rc = pthread_cond_init(&m_cond_queue_empty, NULL);
      if (rc != 0) {
         printf("error: pthread_cond_init failed\n");
         exit(1);
      }

      m_isInitialized = true;
      m_isRunning = true;
        
      /* 
      if ((m_mutex != NULL) &&
          (m_condQueueNotEmpty != NULL) &&
          (m_condQueueNotFull != NULL) &&
          (m_condQueueEmpty != NULL)) {
         m_isInitialized = true;
         m_isRunning = true;
      } else {
         Logger::error("unable to initialize ThreadPoolQueue");
         if (NULL == m_mutex) {
            Logger::error("unable to create mutex");
         }
         if (NULL == m_condQueueNotEmpty) {
            Logger::error("unable to create queue not empty condition variable");
         }
         if (NULL == m_condQueueNotFull) {
            Logger::error("unable to create queue not full condition variable");
         }
         if (NULL == m_condQueueEmpty) {
            Logger::error("unable to create queue empty condition variable");
         }
         printf("error: unable to initialize thread pool queue, aborting\n");
         exit(1);
      }
      */
   } catch (const BasicException& be) {
      Logger::error("exception setting up thread pool queue: " + be.whatString());
   } catch (const std::exception& e) {
      Logger::error("exception setting up thread pool queue: " + std::string(e.what()));
   } catch (...) {
      Logger::error("unknown exception setting up thread pool queue");
   }
}

//******************************************************************************

ThreadPoolQueue::~ThreadPoolQueue() {
   Logger::logInstanceDestroy("ThreadPoolQueue");
   m_isRunning = false;

   /*
   if (NULL != m_mutex) {
      delete m_mutex;
   }
   if (NULL != m_condQueueNotEmpty) {
      delete m_condQueueNotEmpty;
   }
   if (NULL != m_condQueueNotFull) {
      delete m_condQueueNotFull;
   }
   if (NULL != m_condQueueEmpty) {
      delete m_condQueueEmpty;
   }
   */
}

//******************************************************************************

bool ThreadPoolQueue::addRequest(Runnable* runnableRequest) {
   printf("ThreadPoolQueue::addRequest entered\n");

   if (!m_isInitialized) {
      Logger::log(Warning, "ThreadPoolQueue::addRequest queue not initialized");
      printf("addRequest: not initialized, returning false\n");
      return false;
   }

   if (!runnableRequest) {
      Logger::log(Warning, "ThreadPoolQueue::addRequest rejecting NULL request");
      printf("addRequest: runnableRequest is NULL, returning false\n");
      return false;
   }
 
   printf("addRequest, obtaining lock\n"); 
   //MutexLock lock(*m_mutex, "ThreadPoolQueue::addRequest");
   pthread_mutex_lock(&m_queue_lock);
   printf("addRequest, have lock\n");
   
   if (!m_isRunning) {
      Logger::log(Warning, "ThreadPoolQueue::addRequest rejecting request, queue is shutting down");
      printf("addRequest: queue not running, returning false\n");
      pthread_mutex_unlock(&m_queue_lock);
      return false;
   }
   
   //if (!m_mutex->haveValidMutex()) {
   //   Logger::error("don't have valid mutex in addRequest");
   //   ::exit(1);
   //}

   Logger::log(Debug, "ThreadPoolQueue::addRequest accepting request");
   
   const bool wasEmpty = m_queue.empty();
  
   printf("adding new request to queue\n"); 
   // add new request to the queue
   m_queue.push_back(runnableRequest);
   
   // did we just transition from QUEUE_EMPTY to QUEUE_NOT_EMPTY?
   if (wasEmpty) {
      // signal QUEUE_NOT_EMPTY (wake up a worker thread)
      Logger::log(Debug, "signalling queue_not_empty");
      //m_condQueueNotEmpty->notifyAll();
      printf("addRequest: broadcast queue not empty\n");
      pthread_cond_broadcast(&m_cond_queue_not_empty);
   } else {
      printf("addRequest: no transition from empty to not-empty\n");
   }
 
   pthread_mutex_unlock(&m_queue_lock); 
   printf("addRequest returning\n");
   return true;
}

//******************************************************************************

Runnable* ThreadPoolQueue::takeRequest() {
   if (!m_isInitialized) {
      Logger::log(Warning, "ThreadPoolQueue::takeRequest queue not initialized");
      printf("ThreadPoolQueue is not initialized, returning NULL\n");
      return NULL;
   }

   //MutexLock lock(*m_mutex, "ThreadPoolQueue::takeRequest");
   pthread_mutex_lock(&m_queue_lock);

   // is the queue shut down?
   if (!m_isRunning) {
      printf("queue is shut down, returning NULL\n");
      pthread_mutex_unlock(&m_queue_lock);
      return NULL;
   }
   
   //if (!m_mutex->haveValidMutex()) {
   //   Logger::error("don't have valid mutex in takeRequest");
   //   exit(1);
   //}
   
   Runnable* request = NULL;
  
   // is the queue empty?
   while (m_queue.empty()) { // && m_isRunning) {
      // empty queue -- wait for QUEUE_NOT_EMPTY event
      //m_condQueueNotEmpty->wait(m_mutex);
      pthread_cond_wait(&m_cond_queue_not_empty,
                        &m_queue_lock);
   }

   if (!m_queue.empty()) {
      // take a request from the queue
      request = m_queue.front();
      m_queue.pop_front();

      // did we just empty the queue?
      if (m_queue.empty()) {
         printf("we just emptied queue, signal it\n");
         //m_condQueueEmpty->notifyOne();
         pthread_cond_signal(&m_cond_queue_empty);
      }
   } else {
      printf("queue is empty\n");
   }

   pthread_mutex_unlock(&m_queue_lock);

   if (request != NULL) {
      printf("takeRequest returning non-NULL Runnable\n");
   } else {
      printf("takeRequest returning NULL\n");
   }

   return request;
}

//******************************************************************************

bool ThreadPoolQueue::shutDown() {
   bool wasShutDown = false;
   printf("ThreadPoolQueue::shutDown\n");
   if (m_isInitialized && m_isRunning) {
      //MutexLock lock(*m_mutex, "ThreadPoolQueue::shutDown");
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

