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
