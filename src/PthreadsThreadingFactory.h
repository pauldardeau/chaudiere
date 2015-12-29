// Copyright Paul Dardeau, SwampBits LLC 2015
// BSD License

#ifndef CHAUDIERE_PTHREADSTHREADINGFACTORY_H
#define CHAUDIERE_PTHREADSTHREADINGFACTORY_H

#include "ThreadingFactory.h"


namespace chaudiere
{

/**
 * PthreadsThreadingFactory is a factory for creating Pthreads versions
 * of Thread, Mutex, ThreadPoolDispatcher, etc.
 */
class PthreadsThreadingFactory : public ThreadingFactory
{
public:
   /**
    * Constructs a PthreadsThreadingFactory instance
    */
   PthreadsThreadingFactory() noexcept;
   
   /**
    * Destructor
    */
   ~PthreadsThreadingFactory() noexcept;
   
  /**
   * Create a new named PthreadsMutex
   * @param name the name for the new Mutex
   * @return pointer to the newly created Mutex
   * @see Mutex()
   */
  virtual Mutex* createMutex(const std::string& name);
  
  /**
   * Create a new PthreadsThread
   * @return pointer to newly created Thread
   * @see Thread()
   */
  virtual Thread* createThread(const std::string& name) noexcept;
  
  /**
   * Creates a new PthreadsThread to run the specified Runnable
   * @param runnable the Runnable object for the thread to run
   * @return pointer to the newly created Thread
   * @see Thread()
   * @see Runnable()
   */
  virtual Thread* createThread(Runnable* runnable,
                               const std::string& name) noexcept;
  
  /**
   * Create a new PthreadsConditionVariable
   * @return pointer to the newly created ConditionVariable
   * @see ConditionVariable()
   */
  virtual ConditionVariable* createConditionVariable(const std::string& name);
  
  /**
   * Creates a new Pthreads compatible ThreadPool
   * @param numberThreads the number of threads to initialize in the pool dispatcher
   * @return pointer to newly created ThreadPoolDispatcher
   */
  virtual ThreadPoolDispatcher* createThreadPoolDispatcher(int numberThreads,
             const std::string& name) noexcept;
   
private:
   // disallow copies
   PthreadsThreadingFactory(const PthreadsThreadingFactory&);
   PthreadsThreadingFactory& operator=(const PthreadsThreadingFactory&);

};

}

#endif
