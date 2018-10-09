// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_PTHREADSMUTEX_H
#define CHAUDIERE_PTHREADSMUTEX_H

#include <string>
#include <pthread.h>

#include "Mutex.h"

namespace chaudiere
{

/**
 * PthreadsMutex is a wrapper class for working with mutexes from pthreads.
 */
class PthreadsMutex : public Mutex
{
public:
   /**
    * Default constructor
    */
   PthreadsMutex();
   
   /**
    * Constructs mutex with a name
    * @param mutexName the name of the mutex
    */
   explicit PthreadsMutex(const std::string& mutexName);
   
   /**
    * Destructor
    */
   ~PthreadsMutex();

   PthreadsMutex(const PthreadsMutex&) = delete;
   PthreadsMutex& operator=(const PthreadsMutex&) = delete;

   /**
    */
   void initialize();

   /**
    *
    * @return
    */
   virtual bool unlock();
   
   /**
    *
    * @return
    */
   virtual bool lock();
   
   /**
    *
    * @return
    */
   virtual bool isLocked() const;
   
   /**
    *
    * @return
    */
   virtual bool haveValidMutex() const;
   
   /**
    * Retrieves the primitive data type for the underlying platform
    * @return the platform's primitive data type for the mutex
    */
   pthread_mutex_t& getPlatformPrimitive() {
      return m_mutex;
   }
   
   /**
    * Retrieves the name of the mutex
    * @return name of the mutex
    */
   const std::string& getName() const;

    
private:
   pthread_mutex_t m_mutex;
   std::string m_mutexName;
   bool m_haveValidMutex;
   bool m_isLocked;
    
};

}

#endif
