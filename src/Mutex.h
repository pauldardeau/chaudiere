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
   Mutex() {}
   
   /**
    * Destructor
    */
   virtual ~Mutex() {}
   
   /**
    * Unlocks the mutex
    * @return true if mutex was successfully unlocked, false otherwise
    */
   virtual bool unlock() = 0;
   
   /**
    * Locks the mutex
    * @return true if mutex was successfully locked, false otherwise
    */
   virtual bool lock() = 0;
   
   /**
    * Determines if the mutex is currently locked
    * @return true if mutex is locked, false otherwise
    */
   virtual bool isLocked() const = 0;
   
   /**
    * Determines if a valid mutex is present (usable)
    * @return true if valid mutex, false otherwise
    */
   virtual bool haveValidMutex() const = 0;

   virtual const std::string& getName() const = 0;

   
private:
   // copying not allowed
   Mutex(const Mutex&);
   Mutex& operator=(const Mutex&);   
   
};

}

#endif
