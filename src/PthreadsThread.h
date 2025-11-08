// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_PTHREADSTHREAD_H
#define CHAUDIERE_PTHREADSTHREAD_H

#include <pthread.h>
#include <string>

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


      PthreadsThread();
      explicit PthreadsThread(const std::string& name);
      explicit PthreadsThread(Runnable* runnable);
      explicit PthreadsThread(Runnable* runnable, const std::string& name);
      virtual ~PthreadsThread();

      virtual bool start();
      virtual void run();

      unsigned long getExitCode() const;
      pthread_t getHandle();
      const std::string& getName() const;

      virtual void join();


   private:
      // disallow copying
      PthreadsThread(const PthreadsThread&);
      PthreadsThread& operator=(const PthreadsThread&);

      static void* runThread(void* pArgs);
      PthreadsMutex         m_mutexAlive;
      PthreadsMutex         m_mutexExitCode;
      pthread_t             m_threadHandle;
      unsigned long         m_exitCode;
      std::string           m_name;
};

}

#endif

