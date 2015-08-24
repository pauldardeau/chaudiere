// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_THREADPOOL_H
#define CHAUDIERE_THREADPOOL_H

#include <list>
#include <memory>

#include "Thread.h"
#include "ThreadPoolQueue.h"
#include "ThreadPoolWorker.h"
#include "ThreadPoolDispatcher.h"
#include "ThreadingFactory.h"

class Runnable;

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
   explicit ThreadPool(int numberWorkers) noexcept;
   
   /**
    *
    * @param threadingFactory
    * @param numberWorkers
    */
   ThreadPool(ThreadingFactory* threadingFactory, int numberWorkers) noexcept;

   /**
    * Destructor
    */
   ~ThreadPool() noexcept;
   
   // ThreadPoolDispatcher
   /**
    *
    * @return
    */
   virtual bool start() noexcept override;
   
   /**
    *
    * @return
    */
   virtual bool stop() noexcept override;
   
   /**
    *
    * @param runnableRequest
    * @return
    * @see Runnable()
    */
   virtual bool addRequest(Runnable* runnableRequest) noexcept override;
   
   /**
    *
    * @param runnable
    * @return
    * @see Runnable()
    * @see Thread()
    */
   virtual Thread* createThreadWithRunnable(Runnable* runnable) noexcept;
   
   /**
    *
    * @return
    */
   int getNumberWorkers() const noexcept;
   
   /**
    *
    * @param numberNewWorkers
    */
   void addWorkers(int numberNewWorkers) noexcept;
   
   /**
    *
    * @param numberWorkersToRemove
    */
   void removeWorkers(int numberWorkersToRemove) noexcept;
   
   
protected:
   /**
    *
    * @param numberToAddOrDelete
    */
   void adjustNumberWorkers(int numberToAddOrDelete) noexcept;
   
private:
   ThreadingFactory* m_threadingFactory;
   std::list<ThreadPoolWorker*> m_listWorkers;
   ThreadPoolQueue m_queue;
   int m_workerCount;
   int m_workersCreated;
   bool m_isRunning;
   
   // disallow copies
   ThreadPool(const ThreadPool&);
   ThreadPool& operator=(const ThreadPool&);
};

}

#endif
