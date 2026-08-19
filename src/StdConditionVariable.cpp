// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "StdConditionVariable.h"
#include "StdMutex.h"
#include "BasicException.h"
#include "Logger.h"

using namespace chaudiere;

//******************************************************************************

StdConditionVariable::StdConditionVariable() {
   LOG_INSTANCE_CREATE("StdConditionVariable")
}

//******************************************************************************

StdConditionVariable::StdConditionVariable(const std::string& name) :
   m_name(name) {
   LOG_INSTANCE_CREATE("StdConditionVariable")
}

//******************************************************************************

StdConditionVariable::~StdConditionVariable() {
   LOG_INSTANCE_DESTROY("StdConditionVariable")
}

//******************************************************************************

bool StdConditionVariable::wait(Mutex* mutex) {
   if (mutex) {
      StdMutex* stdMutex =
         dynamic_cast<StdMutex*>(mutex);

      if (stdMutex) {
         // the caller already holds this mutex (per the Mutex-caller
         // contract of wait()), so adopt the existing lock rather than
         // attempting to lock it again (std::mutex is non-recursive and
         // a second lock() from the same thread would deadlock).
         std::unique_lock<std::mutex> lock(stdMutex->getPlatformPrimitive(), std::adopt_lock);
         m_cond.wait(lock);
         // std::condition_variable::wait() re-locks the mutex before
         // returning; release() hands that locked state back to the
         // caller (who still owns it, matching StdMutex's own
         // bookkeeping) instead of unlocking it when `lock` goes out of scope.
         lock.release();
         return true;
      } else {
         LOG_ERROR("mutex must be an instance of StdMutex")
      }
   } else {
      LOG_ERROR("no mutex given to wait on")
   }

   return false;
}

//******************************************************************************

void StdConditionVariable::notifyOne() {
   m_cond.notify_one();
}

//******************************************************************************

void StdConditionVariable::notifyAll() {
   m_cond.notify_all();
}

//******************************************************************************

const std::string& StdConditionVariable::getName() const {
   return m_name;
}

//******************************************************************************
