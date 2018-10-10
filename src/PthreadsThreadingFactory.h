// Copyright Paul Dardeau, SwampBits LLC 2015
// BSD License

#ifndef CHAUDIERE_PTHREADSTHREADINGFACTORY_H
#define CHAUDIERE_PTHREADSTHREADINGFACTORY_H

#include <memory>
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
   PthreadsThreadingFactory();

   PthreadsThreadingFactory(const PthreadsThreadingFactory&) = delete;
   PthreadsThreadingFactory& operator=(const PthreadsThreadingFactory&) = delete;

   /**
    * Destructor
    */
   ~PthreadsThreadingFactory();
   
  /**
   * Create a new named PthreadsMutex
   * @param name the name for the new Mutex
   * @return pointer to the newly created Mutex
   * @see Mutex()
   */
  virtual std::unique_ptr<Mutex> createMutex(const std::string& name);
  
  /**
   * Create a new PthreadsThread
   * @return pointer to newly created Thread
   * @see Thread()
   */
  virtual std::unique_ptr<Thread> createThread(const std::string& name);
  
  /**
   * Creates a new PthreadsThread to run the specified Runnable
   * @param runnable the Runnable object for the thread to run
   * @return pointer to the newly created Thread
   * @see Thread()
   * @see Runnable()
   */
  virtual std::unique_ptr<Thread> createThread(Runnable* runnable,
                                               const std::string& name);
  
  /**
   * Create a new PthreadsConditionVariable
   * @return pointer to the newly created ConditionVariable
   * @see ConditionVariable()
   */
  virtual std::unique_ptr<ConditionVariable> createConditionVariable(const std::string& name);
  
  /**
   * Creates a new Pthreads compatible ThreadPool
   * @param numberThreads the number of threads to initialize in the pool dispatcher
   * @return pointer to newly created ThreadPoolDispatcher
   */
  virtual std::unique_ptr<ThreadPoolDispatcher> createThreadPoolDispatcher(int numberThreads,
             const std::string& name);
   
};

}

#endif

