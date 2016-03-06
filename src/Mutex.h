// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_MUTEX_H
#define CHAUDIERE_MUTEX_H

namespace chaudiere
{

/**
 * Mutex is an interface for working with abstract mutexes.
 */
class Mutex
{
public:
   /**
    * Default constructor
    */
   Mutex() noexcept {}
   
   /**
    * Destructor
    */
   virtual ~Mutex() noexcept {}
   
   /**
    * Unlocks the mutex
    * @return true if mutex was successfully unlocked, false otherwise
    */
   virtual bool unlock() noexcept = 0;
   
   /**
    * Locks the mutex
    * @return true if mutex was successfully locked, false otherwise
    */
   virtual bool lock() noexcept = 0;
   
   /**
    * Determines if the mutex is currently locked
    * @return true if mutex is locked, false otherwise
    */
   virtual bool isLocked() const noexcept = 0;
   
   /**
    * Determines if a valid mutex is present (usable)
    * @return true if valid mutex, false otherwise
    */
   virtual bool haveValidMutex() const noexcept = 0;

   virtual const std::string& getName() const noexcept = 0;

   
private:
   // copying not allowed
   Mutex(const Mutex&);
   Mutex& operator=(const Mutex&);   
   
};

}

#endif
