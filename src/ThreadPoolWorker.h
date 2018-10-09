// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_THREADPOOLWORKER_H
#define CHAUDIERE_THREADPOOLWORKER_H

#include <memory>

#include "Runnable.h"


namespace chaudiere
{
   class ThreadingFactory;
   class ThreadPoolQueue;
   class Thread;

/**
 * ThreadPoolWorker provides the logic of pulling work off of a ThreadPoolQueue
 * and processing and then repeating. It's meant to be used within a ThreadPool.
 */
class ThreadPoolWorker : public Runnable
{
   public:
      /**
       *
       * @param threadingFactory
       * @param queue
       * @param workerId
       * @see ThreadingFactory()
       * @see ThreadPoolQueue()
       */
      ThreadPoolWorker(ThreadingFactory* threadingFactory,
                       ThreadPoolQueue& queue,
                       int workerId);

      ThreadPoolWorker(const ThreadPoolWorker&) = delete;
      ThreadPoolWorker& operator=(const ThreadPoolWorker&) = delete;
                       
      /**
       * Destructor
       */
      ~ThreadPoolWorker();

      /**
       *
       */
      void start();
      
      /**
       *
       */
      void stop();
      
      /**
       *
       */
      void run();


   private:
      ThreadingFactory* m_threadingFactory;
      Thread* m_workerThread;
      ThreadPoolQueue& m_poolQueue;
      int m_workerId;
      bool m_isRunning;
};

}

#endif

