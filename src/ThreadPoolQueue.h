// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_THREADPOOLQUEUE_H
#define CHAUDIERE_THREADPOOLQUEUE_H

#include <deque>
#include <memory>


namespace chaudiere
{
   class ConditionVariable;
   class Mutex;
   class Runnable;
   class ThreadingFactory;

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
   explicit ThreadPoolQueue(ThreadingFactory* threadingFactory) noexcept;
   
   /**
    * Destructor
    */
   virtual ~ThreadPoolQueue() noexcept;
   
   /**
    *
    * @param runnableRequest
    * @return
    * @see Runnable()
    */
   virtual bool addRequest(Runnable* runnableRequest) noexcept;
   
   /**
    *
    * @return
    * @see Runnable()
    */
   virtual Runnable* takeRequest() noexcept;
   
   /**
    *
    */
   virtual void shutDown() noexcept;
   
   /**
    *
    * @return
    */
   virtual bool isRunning() const noexcept;
   
   /**
    *
    * @return
    */
   virtual bool isEmpty() const noexcept;
   
   /**
    *
    * @return
    */
   virtual bool isInitialized() const noexcept;
   
   // disallow copies
   ThreadPoolQueue(const ThreadPoolQueue&) = delete;
   ThreadPoolQueue& operator=(const ThreadPoolQueue&) = delete;
   
private:
   ThreadingFactory* m_threadingFactory;
   std::deque<Runnable*> m_queue;
   Mutex* m_mutex;
   ConditionVariable* m_condQueueEmpty;
   ConditionVariable* m_condQueueNotEmpty;
   bool m_isInitialized;
   bool m_isRunning;
};

}

#endif
