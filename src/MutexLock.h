// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_MUTEXLOCK_H
#define CHAUDIERE_MUTEXLOCK_H

#include "Mutex.h"

namespace chaudiere
{

/**
 * MutexLock is a convenience class for locking and unlocking a Mutex. This
 * class is meant to be used with RAII (i.e., on the stack) to get a lock
 * and then have the destructor release the lock.
 */
class MutexLock
{
public:
   explicit MutexLock(Mutex& mutex) :
      m_mutex(mutex),
      m_owns(true) {
      m_mutex.lock();
   }

   /**
    * Locks the given Mutex
    * @param mutex the mutex to lock
    * @see Mutex()
    */
   explicit MutexLock(Mutex& mutex, const std::string& name) :
      m_mutex(mutex),
      m_name(name),
      m_owns(true) {
      m_mutex.lock();
   }

   /**
    * Destructor - unlocks the mutex (if not already unlocked via unlock())
    */
   ~MutexLock() {
      if (m_owns) {
         m_mutex.unlock();
      }
   }

   void unlock() {
      if (m_owns) {
         m_mutex.unlock();
         m_owns = false;
      }
   }

private:
   Mutex& m_mutex;
   std::string m_name;
   bool m_owns;

   MutexLock();
   MutexLock(const Mutex&);
   Mutex& operator=(const Mutex&);

};

}

#endif

