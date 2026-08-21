// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_THREADPOOLQUEUE_H
#define CHAUDIERE_THREADPOOLQUEUE_H

#include <cstddef>
#include <deque>
#include <memory>


namespace chaudiere
{
   class ConditionVariable;
   class Mutex;
   class Runnable;
   class ThreadingFactory;


/**
 * Policy applied by ThreadPoolQueue::addRequest() once the queue has
 * reached its configured maximum size. Only meaningful after
 * setMaxQueueSize() has been called with a non-zero size - by default,
 * the queue is unbounded and this setting has no effect.
 */
enum class QueueFullPolicy {
   Reject,  // addRequest() returns false immediately
   Block    // addRequest() blocks until space is available, or the queue shuts down
};


struct TakeRequestContext {
   Runnable* runnable;
   bool isQueueRunning;
   bool waitIfNone;

   TakeRequestContext() :
      runnable(nullptr),
      isQueueRunning(false),
      waitIfNone(true) {
   }
};


/**
 * ThreadPoolQueue is an abstract base class for a queue being serviced
 * by a thread from a thread pool.
 */
class ThreadPoolQueue
{
public:
   /**
    *
    * @param threadingFactory
    * @see ThreadingFactory()
    */
   explicit ThreadPoolQueue(ThreadingFactory* threadingFactory);

   /**
    * Destructor
    */
   virtual ~ThreadPoolQueue();

   /**
    *
    * @param runnableRequest
    * @return
    * @see Runnable()
    */
   virtual bool addRequest(Runnable* runnableRequest);

   /**
    *
    * @return
    * @see Runnable()
    */
   virtual void takeRequest(TakeRequestContext& ctx);

   /**
    *
    * @return
    */
   virtual bool shutDown();

   /**
    * Reverses a prior shutDown(), allowing the same queue instance to be
    * reused by a freshly started set of workers.
    * @return boolean indicating whether the queue was restarted (false if
    *         it was never initialized, or was already running)
    */
   virtual bool restart();

   /**
    * Sets a maximum number of pending (not-yet-taken) requests the queue
    * will hold, and how addRequest() behaves once that limit is reached.
    * By default the queue is unbounded (maxSize == 0), matching prior
    * behavior. Can be changed at any time, including while requests are
    * being added/taken; a producer currently blocked under the Block
    * policy is woken (and re-checks the new limit) if the limit is
    * raised or removed.
    * @param maxSize maximum number of pending requests (0 == unbounded)
    * @param policy what addRequest() does once the queue is at maxSize
    */
   virtual void setMaxQueueSize(std::size_t maxSize,
                                QueueFullPolicy policy = QueueFullPolicy::Reject);

   /**
    * @return the configured maximum queue size (0 == unbounded)
    */
   virtual std::size_t getMaxQueueSize() const;

   /**
    *
    * @return
    */
   virtual bool isRunning() const;

   /**
    *
    * @return
    */
   virtual bool isEmpty() const;

   /**
    *
    * @return
    */
   virtual bool isInitialized() const;


private:
   ThreadingFactory* m_threadingFactory;
   std::deque<Runnable*> m_queue;

   std::unique_ptr<Mutex> m_mutex;
   std::unique_ptr<ConditionVariable> m_condQueueNotEmpty;
   std::unique_ptr<ConditionVariable> m_condQueueEmpty;
   std::unique_ptr<ConditionVariable> m_condQueueNotFull;

   bool m_isInitialized;
   bool m_isRunning;
   int m_activeTakeRequests;
   int m_activeAddRequests;
   std::size_t m_maxQueueSize;
   QueueFullPolicy m_queueFullPolicy;

   // disallow copies
   ThreadPoolQueue(const ThreadPoolQueue&);
   ThreadPoolQueue& operator=(const ThreadPoolQueue&);

};

}

#endif

