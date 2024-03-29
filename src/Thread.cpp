// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <cstdio>
#include <string>

#include "Thread.h"
#include "ThreadCompletionObserver.h"
#include "MutexLock.h"
#include "BasicException.h"
#include "Logger.h"

static const std::string EMPTY = "";

static const std::string ATTR_THREAD_ID     = "os_tid";
static const std::string ATTR_WORKER_ID     = "worker_id";

using namespace chaudiere;

//******************************************************************************

Thread::Thread() :
   m_runnable(NULL),
   m_isAlive(false),
   m_isPoolWorker(false),
   m_mutexAlive(NULL),
   m_threadCompletionObserver(NULL) {
}

//******************************************************************************

Thread::Thread(Runnable* runnable) :
   m_runnable(runnable),
   m_isAlive(false),
   m_isPoolWorker(false),
   m_mutexAlive(NULL),
   m_threadCompletionObserver(NULL) {
}

//******************************************************************************
/*
Thread::Thread(Mutex* mutexAlive, Runnable* runnable) :
   m_runnable(runnable),
   m_isAlive(false),
   m_isPoolWorker(false),
   m_mutexAlive(mutexAlive),
   m_threadCompletionObserver(NULL) {
   if (Logger::isLogging(Debug)) {
      //Logger::debug("new thread created");
   }
}
*/
//******************************************************************************

Thread::~Thread() {
}

//******************************************************************************

void Thread::setAliveMutex(Mutex* mutexAlive) {
   m_mutexAlive = mutexAlive;
}

//******************************************************************************

void Thread::setPoolWorkerStatus(bool isPoolWorker) {
   m_isPoolWorker = isPoolWorker;
}

//******************************************************************************

bool Thread::isPoolWorker() const {
   return m_isPoolWorker;
}

//******************************************************************************

void Thread::run() {
   // This method should never be called.  If you've subclassed Thread, then
   // you need to implement "void run()" in your derived class.
   throw BasicException("this method should not be called");
}

//******************************************************************************

bool Thread::isAlive() const {
   bool rc;
   
   {
      Thread* pThis = const_cast<Thread*>(this);
      if (NULL != pThis->m_mutexAlive) {
         MutexLock lockMutexAlive(*pThis->m_mutexAlive);
         rc = m_isAlive;
      } else {
         rc = m_isAlive;
      }
   }
   
   return rc;
}

//******************************************************************************

void Thread::setAlive(bool isAlive) {
   if (NULL != m_mutexAlive) {
      MutexLock lockMutexAlive(*m_mutexAlive);
      m_isAlive = isAlive;
   } else {
      m_isAlive = isAlive;
   }
}

//******************************************************************************

void Thread::registerThreadCompletionObserver(ThreadCompletionObserver* observer) {
   m_threadCompletionObserver = observer;
}

//******************************************************************************

void Thread::clearThreadCompletionObserver() {
   m_threadCompletionObserver = NULL;
}

//******************************************************************************

void Thread::notifyOnCompletion() {
   if (m_runnable != NULL) {
      m_runnable->notifyOnCompletion();
   }
   if (m_threadCompletionObserver != NULL) {
      m_threadCompletionObserver->notifyThreadComplete(this);
   }
}

//******************************************************************************

Runnable* Thread::getRunnable() {
   return m_runnable;
}

//******************************************************************************

void Thread::setAttribute(const std::string& key,
                          const std::string& value) {
   m_attributes.addPair(key, value);
}

//******************************************************************************

bool Thread::hasAttribute(const std::string& key) const {
   return m_attributes.hasKey(key);
}

//******************************************************************************

const std::string& Thread::getAttribute(const std::string& key) const {
   // may throw InvalidKeyException
   return m_attributes.getValue(key);
}

//******************************************************************************

void Thread::clearAttribute(const std::string& key) {
   m_attributes.removePair(key);
}

//******************************************************************************

void Thread::setThreadId(const std::string& threadId) {
   setAttribute(ATTR_THREAD_ID, threadId);
}

//******************************************************************************

bool Thread::hasThreadId() const {
   return hasAttribute(ATTR_THREAD_ID);
}

//******************************************************************************

const std::string& Thread::getThreadId() const {
   if (hasThreadId()) {
      return getAttribute(ATTR_THREAD_ID);
   } else {
      return EMPTY;
   }
}

//******************************************************************************

void Thread::setWorkerId(const std::string& workerId) {
   setAttribute(ATTR_WORKER_ID, workerId);
}

//******************************************************************************

bool Thread::hasWorkerId() const {
   return hasAttribute(ATTR_WORKER_ID);
}

//******************************************************************************

const std::string& Thread::getWorkerId() const {
   if (hasWorkerId()) {
      return getAttribute(ATTR_WORKER_ID);
   } else {
      return EMPTY;
   }
}

//******************************************************************************

