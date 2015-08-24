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
   /**
    * Locks the given Mutex
    * @param mutex the mutex to lock
    * @see Mutex()
    */
   explicit MutexLock(Mutex& mutex) noexcept :
      m_mutex(mutex),
      m_isLocked(false)
   {
      if (m_mutex.haveValidMutex()) {
         m_isLocked = m_mutex.lock();
      }
   }

   /**
    * Destructor - unlocks the mutex
    */
   ~MutexLock() noexcept
   {
      unlock();
   }

   /**
    * Unlocks the mutex
    */
   void unlock() noexcept
   {
      if (m_isLocked) {
         if (m_mutex.unlock()) {
            m_isLocked = false;
         }
      }
   }

    
private:
   Mutex& m_mutex;
   bool m_isLocked;
   
   MutexLock();
   MutexLock(const Mutex&);
   Mutex& operator=(const Mutex&);   

};

}

#endif
