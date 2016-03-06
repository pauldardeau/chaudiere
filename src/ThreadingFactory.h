// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_THREADINGFACTORY_H
#define CHAUDIERE_THREADINGFACTORY_H

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
   static ThreadingFactory* getThreadingFactory() noexcept;
   
   /**
    * Sets the specified ThreadingFactory instance to be the class singleton
    * @param threadingFactory the instance to use for the new class singleton
    */
   static void setThreadingFactory(ThreadingFactory* threadingFactory) noexcept;

   /**
    * Constructs a ThreadingFactory instance
    */
   ThreadingFactory() noexcept {}
   
   /**
    * Destructor
    */
   virtual ~ThreadingFactory() noexcept {}
   
   /**
    * Create a new named Mutex
    * @param name the name for the new Mutex
    * @return pointer to the newly created Mutex
    * @see Mutex()
    */
   virtual Mutex* createMutex(const std::string& name) = 0;
   
   /**
    * Create a new Thread
    * @return pointer to newly created Thread
    * @see Thread()
    */
   virtual Thread* createThread(const std::string& name) = 0;
   
   /**
    * Creates a new Thread to run the specified Runnable
    * @param runnable the Runnable object for the thread to run
    * @return pointer to the newly created Thread
    * @see Thread()
    * @see Runnable()
    */
   virtual Thread* createThread(Runnable* runnable, const std::string& name) = 0;
   
   /**
    * Create a new ConditionVariable
    * @return pointer to the newly created ConditionVariable
    * @see ConditionVariable()
    */
   virtual ConditionVariable* createConditionVariable(const std::string& name) = 0;
   
   /**
    * Creates a new ThreadPoolDispatcher
    * @param numberThreads the number of threads to initialize in the pool dispatcher
    * @return pointer to newly created ThreadPoolDispatcher
    */
   virtual ThreadPoolDispatcher* createThreadPoolDispatcher(int numberThreads,
            const std::string& name) = 0;
   

private:
   // disallow copies
   ThreadingFactory(const ThreadingFactory&);
   ThreadingFactory& operator=(const ThreadingFactory&);

   static ThreadingFactory* threadingFactoryInstance;

};

}

#endif
