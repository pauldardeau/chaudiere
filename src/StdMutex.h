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
   ~StdMutex() noexcept;
   
   // Mutex
   /**
    *
    * @return
    */
   virtual bool unlock() noexcept override;
   
   /**
    *
    * @return
    */
   virtual bool lock() noexcept override;
   
   /**
    *
    * @return
    */
   virtual bool isLocked() const noexcept override;
   
   /**
    *
    * @return
    */
   virtual bool haveValidMutex() const noexcept override;
   
   /**
    *
    * @return
    */
   std::mutex& getPlatformPrimitive() noexcept
   {
      return m_mutex;
   }
   
   /**
    *
    * @return
    */
   const std::string& getName() const noexcept;
   
   
   
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
