// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <stdio.h>
#include <string>

#include "PthreadsThread.h"
#include "MutexLock.h"
#include "BasicException.h"
#include "Logger.h"

using namespace chaudiere;

//******************************************************************************

PthreadsThread::PthreadsThread() :
   Thread(nullptr),
   m_mutexAlive("mutexAlive"),
   m_mutexExitCode("mutexExitCode"),
   m_threadHandle(0),
   m_exitCode(1L),
   m_name("") {
   LOG_INSTANCE_CREATE("PthreadsThread")
   //if (m_mutexAlive.haveValidMutex()) {
   //   setAliveMutex(&m_mutexAlive);
   //}
}

//******************************************************************************

PthreadsThread::PthreadsThread(const std::string& name) :
   Thread(nullptr),
   m_mutexAlive("mutexAlive"),
   m_mutexExitCode("mutexExitCode"),
   m_threadHandle(0),
   m_exitCode(1L),
   m_name(name) {
   LOG_INSTANCE_CREATE("PthreadsThread")
   //if (m_mutexAlive.haveValidMutex()) {
   //   setAliveMutex(&m_mutexAlive);
   //}
}

//******************************************************************************

PthreadsThread::PthreadsThread(Runnable* runnable) :
   Thread(runnable),
   m_mutexAlive("mutexAlive"),
   m_mutexExitCode("mutexExitCode"),
   m_threadHandle(0),
   m_exitCode(1L),
   m_name("") {
   LOG_INSTANCE_CREATE("PthreadsThread")
   //if (m_mutexAlive.haveValidMutex()) {
   //   setAliveMutex(&m_mutexAlive);
   //}
}

//******************************************************************************

PthreadsThread::PthreadsThread(Runnable* runnable, const std::string& name) :
   Thread(runnable),
   m_mutexAlive("mutexAlive"),
   m_mutexExitCode("mutexExitCode"),
   m_threadHandle(0),
   m_exitCode(1L),
   m_name(name) {
   LOG_INSTANCE_CREATE("PthreadsThread")
   //if (m_mutexAlive.haveValidMutex()) {
   //   setAliveMutex(&m_mutexAlive);
   //}
}

//******************************************************************************

PthreadsThread::~PthreadsThread() {
   LOG_INSTANCE_DESTROY("PthreadsThread")
}

//******************************************************************************

void* PthreadsThread::runThread(void* pArgs) {
   PthreadsThread* pThread = (PthreadsThread*) pArgs;

   if (nullptr == pThread) {
      return (void*) 1L;
   }

   char threadId[128];
   ::snprintf(threadId, 128, "%lx", (long unsigned int) pThread);
   pThread->setThreadId(threadId);

   unsigned long rc = 0L;

   try {
      pThread->setAlive(true);

      // if we had a runnable passed in on the constructor, use it.  otherwise,
      // call "run" on the thread object itself.

      Runnable* runnable = pThread->getRunnable();

      if (nullptr != runnable) {
         runnable->run();
      } else {
         pThread->run();
      }
   } catch (...) {
      rc = 1L;
      LOG_ERROR("Thread::runThread exception caught running thread")
   }

   pThread->setAlive(false);
   pThread->notifyOnCompletion();

   return (void*) rc;
}

//******************************************************************************

bool PthreadsThread::start() {
   bool isSuccess = false;

   if (0 == ::pthread_create(&m_threadHandle, 0, runThread, (void*) this)) {
      isSuccess = true;
   }

   return isSuccess;
}

//******************************************************************************

void PthreadsThread::run() {
   // This method should never be called.  If you've subclassed Thread, then
   // you need to implement "void run()" in your derived class.
   printf("PthreadsThread::run - this method should never be called\n");
   throw BasicException("this method should not be called");
}

//******************************************************************************

unsigned long PthreadsThread::getExitCode() const {
   unsigned long rc;

   {
      //PthreadsThread* pThis = const_cast<PthreadsThread*>(this);
      //MutexLock lockMutexExitCode(pThis->m_mutexExitCode);
      rc = m_exitCode;
   }

   return rc;
}

//******************************************************************************

pthread_t PthreadsThread::getHandle() {
   return m_threadHandle;
}

//******************************************************************************

const std::string& PthreadsThread::getName() const {
   return m_name;
}

//******************************************************************************

void PthreadsThread::join() {
   pthread_join(m_threadHandle, nullptr);
}

//******************************************************************************

