// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_THREADINGFACTORY_H
#define CHAUDIERE_THREADINGFACTORY_H

#include <memory>
#include <string>

namespace chaudiere
{
   class Mutex;
   class Thread;
   class ConditionVariable;
   class Runnable;
   class ThreadPoolDispatcher;

/**
 * ThreadingFactory is a factory for creating Thread, Mutex, and ThreadPoolDispatcher
 * instances.
 */
class ThreadingFactory
{
public:
   /**
    * Retrieves the class singleton instance
    * @return the class singleton instance
    */
   static ThreadingFactory* getThreadingFactory();
   
   /**
    * Sets the specified ThreadingFactory instance to be the class singleton
    * @param threadingFactory the instance to use for the new class singleton
    */
   static void setThreadingFactory(ThreadingFactory* threadingFactory);

   /**
    * Constructs a ThreadingFactory instance
    */
   ThreadingFactory() {}

   ThreadingFactory(const ThreadingFactory&) = delete;
   ThreadingFactory& operator=(const ThreadingFactory&) = delete;
   
   /**
    * Destructor
    */
   virtual ~ThreadingFactory() {}
   
   /**
    * Create a new named Mutex
    * @param name the name for the new Mutex
    * @return pointer to the newly created Mutex
    * @see Mutex()
    */
   virtual std::unique_ptr<Mutex> createMutex(const std::string& name) = 0;
   
   /**
    * Create a new Thread
    * @return pointer to newly created Thread
    * @see Thread()
    */
   virtual std::unique_ptr<Thread> createThread(const std::string& name) = 0;
   
   /**
    * Creates a new Thread to run the specified Runnable
    * @param runnable the Runnable object for the thread to run
    * @return pointer to the newly created Thread
    * @see Thread()
    * @see Runnable()
    */
   virtual std::unique_ptr<Thread> createThread(Runnable* runnable, const std::string& name) = 0;
   
   /**
    * Create a new ConditionVariable
    * @return pointer to the newly created ConditionVariable
    * @see ConditionVariable()
    */
   virtual std::unique_ptr<ConditionVariable> createConditionVariable(const std::string& name) = 0;
   
   /**
    * Creates a new ThreadPoolDispatcher
    * @param numberThreads the number of threads to initialize in the pool dispatcher
    * @return pointer to newly created ThreadPoolDispatcher
    */
   virtual std::unique_ptr<ThreadPoolDispatcher> createThreadPoolDispatcher(int numberThreads,
            const std::string& name) = 0;
   

private:
   static ThreadingFactory* threadingFactoryInstance;

};

}

#endif
