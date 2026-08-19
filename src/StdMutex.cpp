// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "StdMutex.h"
#include "Logger.h"

static const std::string EMPTY_STRING = "";

using namespace chaudiere;

//******************************************************************************

StdMutex::StdMutex() :
   StdMutex(EMPTY_STRING) {
}

//******************************************************************************

StdMutex::StdMutex(const std::string& mutexName) :
   m_mutexName(mutexName),
   m_isLocked(false) {
   LOG_INSTANCE_CREATE("StdMutex")
}

//******************************************************************************

StdMutex::~StdMutex() {
   LOG_INSTANCE_DESTROY("StdMutex")

   if (m_isLocked) {
      unlock();
   }
}

//******************************************************************************

bool StdMutex::unlock() {
   // Deliberately not gated on m_isLocked: see the identical comment in
   // PthreadsMutex::unlock(). This mutex can be shared across threads via
   // a ConditionVariable, whose wait() adopts/releases the underlying
   // std::mutex directly (bypassing m_isLocked), so the bookkeeping flag
   // can be stale relative to which thread actually holds the real lock.
   // Gating on it can skip the real unlock() call and deadlock permanently.
   //
   // m_isLocked is std::atomic<bool> specifically because it's read and
   // written from whichever threads lock()/unlock() this object (and by
   // isLocked() from any thread), which a plain bool cannot do safely
   // (confirmed as a real data race with ThreadSanitizer).
   m_mutex.unlock();
   m_isLocked = false;
   return true;
}

//******************************************************************************

bool StdMutex::lock() {
   m_mutex.lock();
   m_isLocked = true;
   return true;
}

//******************************************************************************

bool StdMutex::isLocked() const {
   return m_isLocked;
}

//******************************************************************************

bool StdMutex::haveValidMutex() const {
   return true;
}

//******************************************************************************

const std::string& StdMutex::getName() const {
   return m_mutexName;
}

//******************************************************************************

