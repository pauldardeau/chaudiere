// Copyright Paul Dardeau, SwampBits LLC 2015
// BSD License

#ifndef CHAUDIERE_STDTHREADINGFACTORY_H
#define CHAUDIERE_STDTHREADINGFACTORY_H

#include "ThreadingFactory.h"


namespace chaudiere
{

/**
 * StdThreadingFactory is a factory for creating c++11 versions
 * of Thread, Mutex, ThreadPoolDispatcher, etc.
 */
class StdThreadingFactory : public ThreadingFactory
{
public:
   /**
    * Constructs a StdThreadingFactory instance
    */
   StdThreadingFactory() noexcept;
   
   /**
    * Destructor
    */
   ~StdThreadingFactory() noexcept;

  /**
   * Create a new StdMutex
   * @return pointer to newly created Mutex
   * @see Mutex()
   */
  virtual Mutex* createMutex();
  
  /**
   * Create a new named StdMutex
   * @param mutexName the name for the new Mutex
   * @return pointer to the newly created Mutex
   * @see Mutex()
   */
  virtual Mutex* createMutex(const std::string& mutexName);
  
  /**
   * Create a new StdThread
   * @return pointer to newly created Thread
   * @see Thread()
   */
  virtual Thread* createThread() noexcept;
  
  /**
   * Creates a new StdThread to run the specified Runnable
   * @param runnable the Runnable object for the thread to run
   * @return pointer to the newly created Thread
   * @see Thread()
   * @see Runnable()
   */
  virtual Thread* createThread(Runnable* runnable) noexcept;
  
  /**
   * Create a new StdConditionVariable
   * @return pointer to the newly created ConditionVariable
   * @see ConditionVariable()
   */
  virtual ConditionVariable* createConditionVariable();
  
  /**
   * Creates a new Std C++11 compatible ThreadPool
   * @param numberThreads the number of threads to initialize in the pool dispatcher
   * @return pointer to newly created ThreadPoolDispatcher
   */
  virtual ThreadPoolDispatcher* createThreadPoolDispatcher(int numberThreads) noexcept;

   
private:
   // disallow copies
   StdThreadingFactory(const StdThreadingFactory&);
   StdThreadingFactory& operator=(const StdThreadingFactory&);

};

}

#endif
