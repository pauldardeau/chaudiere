// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_STDMUTEX_H
#define CHAUDIERE_STDMUTEX_H

#include <string>
#include <mutex>

#include "Mutex.h"


namespace chaudiere
{

/**
 * StdMutex is a Mutex that is implemented as a wrapper around std::mutex (C++ 11).
 */
class StdMutex : public Mutex
{
public:
   /**
    * Default constructor
    */
   StdMutex();

   /**
    *
    * @param mutexName
    */
   explicit StdMutex(const std::string& mutexName);

   /**
    * Destructor
    */
   ~StdMutex();

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
    *
    * @return
    */
   std::mutex& getPlatformPrimitive()
   {
      return m_mutex;
   }

   /**
    *
    * @return
    */
   const std::string& getName() const;



private:
   // copying not allowed
   StdMutex(const StdMutex&);
   StdMutex& operator=(const StdMutex&);

   std::mutex  m_mutex;
   std::string m_mutexName;
   bool m_isLocked;

};

}

#endif
