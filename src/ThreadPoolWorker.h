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
                       int workerId) noexcept;
                       
      /**
       * Destructor
       */
      ~ThreadPoolWorker() noexcept;

      /**
       *
       */
      void start() noexcept;
      
      /**
       *
       */
      void stop() noexcept;
      
      /**
       *
       */
      void run() noexcept override;


   private:
      ThreadingFactory* m_threadingFactory;
      Thread* m_workerThread;
      ThreadPoolQueue& m_poolQueue;
      int m_workerId;
      bool m_isRunning;
      
      // disallow copies
      ThreadPoolWorker(const ThreadPoolWorker&);
      ThreadPoolWorker& operator=(const ThreadPoolWorker&);

};

}

#endif

