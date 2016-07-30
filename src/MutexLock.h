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
   explicit MutexLock(Mutex& mutex) noexcept :
      m_mutex(mutex) {
      m_mutex.lock();
   }

   /**
    * Locks the given Mutex
    * @param mutex the mutex to lock
    * @see Mutex()
    */
   explicit MutexLock(Mutex& mutex, const std::string& name) noexcept :
      m_mutex(mutex),
      m_name(name) {
      m_mutex.lock();
   }

   /**
    * Destructor - unlocks the mutex
    */
   ~MutexLock() noexcept {
      m_mutex.unlock();
   }

   void unlock() noexcept {
      m_mutex.unlock();
   }
    
private:
   Mutex& m_mutex;
   std::string m_name;
   
   MutexLock();
   MutexLock(const Mutex&);
   Mutex& operator=(const Mutex&);   

};

}

#endif

