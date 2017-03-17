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
   virtual Runnable* takeRequest();
   
   /**
    *
    */
   virtual void shutDown();
   
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
   Mutex* m_mutex;
   ConditionVariable* m_condQueueNotEmpty;
   bool m_isInitialized;
   bool m_isRunning;

   // disallow copies
   ThreadPoolQueue(const ThreadPoolQueue&);
   ThreadPoolQueue& operator=(const ThreadPoolQueue&);

};

}

#endif
