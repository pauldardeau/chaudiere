// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_THREADPOOL_H
#define CHAUDIERE_THREADPOOL_H

#include <string>
#include <list>
#include <memory>

#include "Thread.h"
#include "ThreadPoolQueue.h"
#include "ThreadPoolWorker.h"
#include "ThreadPoolDispatcher.h"
#include "ThreadingFactory.h"

namespace chaudiere
{

/**
 * ThreadPool is an interface (abstract base class) for thread pools.
 */
class ThreadPool : public ThreadPoolDispatcher
{
public:
   /**
    *
    * @param numberWorkers
    */
   explicit ThreadPool(int numberWorkers);

   explicit ThreadPool(int numberWorkers, const std::string& name);

   /**
    *
    * @param threadingFactory
    * @param numberWorkers
    */
   ThreadPool(ThreadingFactory* threadingFactory, int numberWorkers);

   ThreadPool(ThreadingFactory* threadingFactory, int numberWorkers,
              const std::string& name);

   /**
    * Destructor
    */
   ~ThreadPool();

   // ThreadPoolDispatcher
   /**
    *
    * @return
    */
   virtual bool start();

   /**
    *
    * @return
    */
   virtual bool stop();

   /**
    *
    * @param runnableRequest
    * @return
    * @see Runnable()
    */
   virtual bool addRequest(Runnable* runnableRequest);

   /**
    *
    * @param runnable
    * @return
    * @see Runnable()
    * @see Thread()
    */
   virtual Thread* createThreadWithRunnable(Runnable* runnable);

   /**
    *
    * @return
    */
   int getNumberWorkers() const;

   /**
    *
    * @param numberNewWorkers
    */
   bool addWorkers(int numberNewWorkers);

   /**
    *
    * @param numberWorkersToRemove
    */
   bool removeWorkers(int numberWorkersToRemove);

   const std::string& getName() const;

   bool isRunning() const;

   /**
    * Sets a maximum number of pending (not-yet-taken) requests the pool's
    * queue will hold, and how addRequest() behaves once that limit is
    * reached. By default the queue is unbounded (maxSize == 0). An
    * undersized pool with an unbounded queue can accept unlimited work
    * it has no capacity to keep up with; a max size gives it a way to
    * push back instead.
    * @param maxSize maximum number of pending requests (0 == unbounded)
    * @param policy what addRequest() does once the queue is at maxSize
    * @see QueueFullPolicy
    */
   void setMaxQueueSize(std::size_t maxSize,
                        QueueFullPolicy policy = QueueFullPolicy::Reject);

   /**
    * @return the configured maximum queue size (0 == unbounded)
    */
   std::size_t getMaxQueueSize() const;


protected:
   /**
    *
    * @param numberToAddOrDelete
    */
   bool adjustNumberWorkers(int numberToAddOrDelete);

private:
   ThreadingFactory* m_threadingFactory;
   std::list<ThreadPoolWorker*> m_listWorkers;
   ThreadPoolQueue m_queue;
   int m_workerCount;
   int m_workersCreated;
   bool m_isRunning;
   std::string m_name;

   // disallow copies
   ThreadPool(const ThreadPool&);
   ThreadPool& operator=(const ThreadPool&);
};

}

#endif
