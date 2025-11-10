// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_THREAD_H
#define CHAUDIERE_THREAD_H


#include "Runnable.h"
#include "KeyValuePairs.h"


namespace chaudiere {

class Mutex;
class ThreadCompletionObserver;

/**
 * Thread is an abstract base class for working with threads.
 */
class Thread : public Runnable {

public:
   /**
    *
    * @param mutexAlive
    */
   Thread();

   /**
    *
    * @param runnable
    */
   Thread(Runnable* runnable);

   /**
    *
    * @param mutexAlive
    * @param runnable
    */
   //Thread(Mutex* mutexAlive, Runnable* runnable);

   /**
    * Destructor
    */
   virtual ~Thread();

   void setAliveMutex(Mutex* mutexAlive);


   /**
    *
    * @param isPoolWorker
    */
   void setPoolWorkerStatus(bool isPoolWorker);

   /**
    *
    * @return
    */
   bool isPoolWorker() const;

   /**
    *
    * @return
    */
   virtual bool start() = 0;

   /**
    *
    */
   virtual void run();

   /**
    *
    * @return
    */
   bool isAlive() const;

   /**
    *
    * @param isAlive
    */
   void setAlive(bool isAlive);

   /**
    *
    * @param observer
    */
   void registerThreadCompletionObserver(ThreadCompletionObserver* observer);

   /**
    *
    */
   void clearThreadCompletionObserver();

   /**
    *
    */
   void notifyOnCompletion();


   /**
    *
    * @return
    */
   Runnable* getRunnable();


   /**
    *
    * @param key
    * @param value
    */
   void setAttribute(const std::string& key,
                     const std::string& value);

   /**
    *
    * @param key
    * @return
    */
   bool hasAttribute(const std::string& key) const;

   /**
    *
    * @param key
    * @return
    * @throws InvalidKeyException
    */
   const std::string& getAttribute(const std::string& key) const;

   /**
    *
    * @param key
    */
   void clearAttribute(const std::string& key);

   /**
    *
    * @param threadId
    */
   void setThreadId(const std::string& threadId);

   /**
    *
    * @return
    */
   bool hasThreadId() const;

   /**
    *
    * @return
    */
   const std::string& getThreadId() const;

   /**
    *
    * @param workerId
    */
   void setWorkerId(const std::string& workerId);

   /**
    *
    * @return
    */
   bool hasWorkerId() const;

   /**
    *
    * @return
    */
   const std::string& getWorkerId() const;

   virtual void join() = 0;

   static void sleep(long msec);


private:
   Runnable* m_runnable;
   bool m_isAlive;
   bool m_isPoolWorker;
   Mutex* m_mutexAlive; // weak
   ThreadCompletionObserver* m_threadCompletionObserver;
   KeyValuePairs m_attributes;

   // disallow copying
   Thread(const Thread&);
   Thread& operator=(const Thread&);

};

}

#endif

