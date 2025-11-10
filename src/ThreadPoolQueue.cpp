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
#include "Thread.h"

using namespace chaudiere;


//******************************************************************************

ThreadPoolQueue::ThreadPoolQueue(ThreadingFactory* threadingFactory) :
   m_threadingFactory(threadingFactory),
   m_mutex(m_threadingFactory->createMutex("ThreadPoolQueue")),
   m_condQueueNotEmpty(m_threadingFactory->createConditionVariable("queue-not-empty")),
   m_condQueueEmpty(m_threadingFactory->createConditionVariable("queue-empty")),
   m_isInitialized(false),
   m_isRunning(false),
   m_activeTakeRequests(0),
   m_activeAddRequests(0) {

   LOG_INSTANCE_CREATE("ThreadPoolQueue")

   try {
      if (m_mutex && m_condQueueNotEmpty && m_condQueueEmpty) {
         m_isInitialized = true;
         m_isRunning = true;
      } else {
         LOG_ERROR("unable to initialize ThreadPoolQueue")
         if (!m_mutex) {
            LOG_ERROR("unable to create mutex")
         }
         if (!m_condQueueNotEmpty) {
            LOG_ERROR("unable to create queue not empty condition variable")
         }
         if (!m_condQueueEmpty) {
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

   shutDown();

   while (m_activeAddRequests > 0 || m_activeTakeRequests > 0) {
#if defined(DEBUG)
      printf("ThreadPoolQueue::~ThreadPoolQueue  active adds=%d, active takes=%d\n",
             m_activeAddRequests, m_activeTakeRequests);
#endif
      Thread::sleep(3);
   }

#if defined(DEBUG)
   printf("ThreadPoolQueue::~ThreadPoolQueue  active adds=%d, active takes=%d\n",
          m_activeAddRequests, m_activeTakeRequests);
#endif
}

//******************************************************************************

bool ThreadPoolQueue::addRequest(Runnable* runnableRequest) {
   if (!m_isInitialized) {
      LOG_WARNING("ThreadPoolQueue::addRequest queue not initialized")
      return false;
   }

   if (nullptr == runnableRequest) {
      LOG_WARNING("ThreadPoolQueue::addRequest rejecting nullptr request")
      return false;
   }
 
   MutexLock lock(*m_mutex, "ThreadPoolQueue::addRequest");

   ++m_activeAddRequests;
   
   if (!m_isRunning) {
      LOG_WARNING("ThreadPoolQueue::addRequest rejecting request, queue is shutting down")
      --m_activeAddRequests;
      return false;
   }
   
   if (!m_mutex->haveValidMutex()) {
      LOG_ERROR("don't have valid mutex in addRequest")
      ::exit(1);
   }

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

   --m_activeAddRequests;
 
   return true;
}

//******************************************************************************

void ThreadPoolQueue::takeRequest(TakeRequestContext& ctx) {
   if (!m_isInitialized) {
      LOG_WARNING("ThreadPoolQueue::takeRequest queue not initialized")
      ctx.runnable = nullptr;
      ctx.isQueueRunning = false;
      return;
   }

   MutexLock lock(*m_mutex, "ThreadPoolQueue::takeRequest");

   // is the queue shut down?
   if (!m_isRunning) {
      ctx.runnable = nullptr;
      ctx.isQueueRunning = false;
      return;
   }
   
   if (!m_mutex->haveValidMutex()) {
      LOG_ERROR("don't have valid mutex in takeRequest")
      exit(1);
   }

   ++m_activeTakeRequests;

   if (ctx.waitIfNone) {
      // is the queue empty?
      while (m_queue.empty() && m_isRunning) {
         // empty queue -- wait for QUEUE_NOT_EMPTY event
#if defined(DEBUG)
         printf("ThreadPoolQueue::takeRequest - waiting on queue-not-empty\n");
#endif
         m_condQueueNotEmpty->wait(m_mutex.get());
      }
   } else {
#if defined(DEBUG)
      printf("ThreadPoolQueue::takeRequest - not waiting\n");
#endif
   }

   if (!m_isRunning) {
#if defined(DEBUG)
      printf("ThreadPoolQueue::takeRequest - queue not running\n");
#endif
      ctx.runnable = nullptr;
      ctx.isQueueRunning = false;
   } else {
      ctx.isQueueRunning = true;

      if (!m_queue.empty()) {
         // take a request from the queue
#if defined(DEBUG)
         printf("ThreadPoolQueue::takeRequest - have request from queue\n");
#endif
         ctx.runnable = m_queue.front();
         m_queue.pop_front();

         // did we just empty the queue?
         if (m_queue.empty()) {
#if defined(DEBUG)
            printf("ThreadPoolQueue::takeRequest - emptied queue - notifying queue-empty\n");
#endif
            m_condQueueEmpty->notifyOne();
         }
      } else {
#if defined(DEBUG)
         printf("ThreadPoolQueue::takeRequest - no runnable found in queue - returning nullptr\n");
#endif
         ctx.runnable = nullptr;
      }
   }

   --m_activeTakeRequests;
}

//******************************************************************************

bool ThreadPoolQueue::shutDown() {
   bool wasShutDown = false;

#if defined(DEBUG)
   printf("ThreadPoolQueue::shutdown called\n");
#endif

   if (m_isInitialized && m_isRunning) {
      MutexLock lock(*m_mutex, "ThreadPoolQueue::shutDown");

      m_isRunning = false;
      wasShutDown = true;

#if defined(DEBUG)
      printf("ThreadPoolQueue::shutdown - m_isRunning now false\n");
#endif

      // wake up workers so that they can exit
      m_condQueueNotEmpty->notifyAll();
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

