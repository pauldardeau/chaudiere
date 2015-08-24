// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_PTHREADSTHREAD_H
#define CHAUDIERE_PTHREADSTHREAD_H

#include <pthread.h>

#include "Thread.h"
#include "PthreadsMutex.h"

namespace chaudiere
{

/**
 * PthreadsThread is a wrapper class for working with POSIX threads (pthreads).
 */
class PthreadsThread : public Thread
{
   public:
      typedef void* THREAD_RETURN_T;
      typedef THREAD_RETURN_T (*PFN_THREAD_ROUTINE)(void*);


      PthreadsThread() noexcept;
      explicit PthreadsThread(Runnable* runnable) noexcept;
      virtual ~PthreadsThread() noexcept;

      virtual bool start() noexcept override;
      virtual void run() override;

      unsigned long getExitCode() const noexcept;
      pthread_t getHandle() noexcept;


   private:
      // disallow copying
      PthreadsThread(const PthreadsThread&);
      PthreadsThread& operator=(const PthreadsThread&);

      static void* runThread(void* pArgs) noexcept;
      PthreadsMutex         m_mutexAlive;
      PthreadsMutex         m_mutexExitCode;
      pthread_t             m_threadHandle;
      unsigned long         m_exitCode;
};

}

#endif

