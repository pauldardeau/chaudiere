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
   m_condQueueNotFull(m_threadingFactory->createConditionVariable("queue-not-full")),
   m_isInitialized(false),
   m_isRunning(false),
   m_activeTakeRequests(0),
   m_activeAddRequests(0),
   m_maxQueueSize(0),
   m_queueFullPolicy(QueueFullPolicy::Reject) {

   LOG_INSTANCE_CREATE("ThreadPoolQueue")

   try {
      if (m_mutex && m_condQueueNotEmpty && m_condQueueEmpty && m_condQueueNotFull) {
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
         if (!m_condQueueNotFull) {
            LOG_ERROR("unable to create queue not full condition variable")
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

   if ((m_maxQueueSize > 0) && (m_queue.size() >= m_maxQueueSize)) {
      if (m_queueFullPolicy == QueueFullPolicy::Reject) {
         LOG_WARNING("ThreadPoolQueue::addRequest rejecting request, queue is full")
         --m_activeAddRequests;
         return false;
      }

      // Block policy: wait for a slot to free up. Re-checks m_maxQueueSize
      // on every wakeup, since setMaxQueueSize() may have raised or
      // removed the limit while we were waiting.
      while ((m_maxQueueSize > 0) && (m_queue.size() >= m_maxQueueSize) && m_isRunning) {
         LOG_DEBUG("ThreadPoolQueue::addRequest - waiting on queue-not-full")
         m_condQueueNotFull->wait(m_mutex.get());
      }

      if (!m_isRunning) {
         LOG_WARNING("ThreadPoolQueue::addRequest rejecting request, queue is shutting down")
         --m_activeAddRequests;
         return false;
      }
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

         // a slot just freed up - wake one producer blocked under the
         // Block policy, if any. Harmless no-op when the queue is
         // unbounded or no one is waiting.
         m_condQueueNotFull->notifyOne();
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

      // wake up any producer blocked in addRequest() under the Block
      // policy, so it can see m_isRunning is now false and return
      // rather than waiting forever
      m_condQueueNotFull->notifyAll();
   }

   return wasShutDown;
}

//******************************************************************************

bool ThreadPoolQueue::restart() {
   bool wasRestarted = false;

   if (m_isInitialized && !m_isRunning) {
      MutexLock lock(*m_mutex, "ThreadPoolQueue::restart");

      m_isRunning = true;
      wasRestarted = true;
   }

   return wasRestarted;
}

//******************************************************************************

void ThreadPoolQueue::setMaxQueueSize(std::size_t maxSize, QueueFullPolicy policy) {
   MutexLock lock(*m_mutex, "ThreadPoolQueue::setMaxQueueSize");

   m_maxQueueSize = maxSize;
   m_queueFullPolicy = policy;

   // raising or removing the limit may unblock a producer waiting under
   // the Block policy
   m_condQueueNotFull->notifyAll();
}

//******************************************************************************

std::size_t ThreadPoolQueue::getMaxQueueSize() const {
   return m_maxQueueSize;
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

