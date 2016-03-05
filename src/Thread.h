// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_THREAD_H
#define CHAUDIERE_THREAD_H


#include "Runnable.h"
#include "KeyValuePairs.h"


namespace chaudiere
{
   class Mutex;
   class ThreadCompletionObserver;

/**
 * Thread is an abstract base class for working with threads.
 */
class Thread : public Runnable
{
public:
   /**
    *
    * @param mutexAlive
    */
   explicit Thread(Mutex& mutexAlive) noexcept;
   
   /**
    *
    * @param mutexAlive
    * @param runnable
    */
   Thread(Mutex& mutexAlive, Runnable* runnable) noexcept;
   
   /**
    * Destructor
    */
   virtual ~Thread() noexcept;
   
   
   /**
    *
    * @param isPoolWorker
    */
   void setPoolWorkerStatus(bool isPoolWorker) noexcept;
   
   /**
    *
    * @return
    */
   bool isPoolWorker() const noexcept;
   
   /**
    *
    * @return
    */
   virtual bool start() noexcept = 0;
   
   /**
    *
    */
   virtual void run() override;
   
   /**
    *
    * @return
    */
   bool isAlive() const noexcept;
   
   /**
    *
    * @param isAlive
    */
   void setAlive(bool isAlive) noexcept;
   
   /**
    *
    * @param observer
    */
   void registerThreadCompletionObserver(ThreadCompletionObserver* observer) noexcept;
   
   /**
    *
    */
   void clearThreadCompletionObserver() noexcept;
   
   /**
    *
    */
   void notifyOnCompletion() noexcept override;
   
   
   /**
    *
    * @return
    */
   Runnable* getRunnable() noexcept;
   
   
   /**
    *
    * @param key
    * @param value
    */
   void setAttribute(const std::string& key,
                     const std::string& value) noexcept;
   
   /**
    *
    * @param key
    * @return
    */
   bool hasAttribute(const std::string& key) const noexcept;
   
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
   void clearAttribute(const std::string& key) noexcept;
   
   /**
    *
    * @param threadId
    */
   void setThreadId(const std::string& threadId) noexcept;
   
   /**
    *
    * @return
    */
   bool hasThreadId() const noexcept;
   
   /**
    *
    * @return
    */
   const std::string& getThreadId() const;
   
   /**
    *
    * @param workerId
    */
   void setWorkerId(const std::string& workerId) noexcept;
   
   /**
    *
    * @return
    */
   bool hasWorkerId() const noexcept;
   
   /**
    *
    * @return
    */
   const std::string& getWorkerId() const;

   
private:
   Runnable* m_runnable;
   bool m_isAlive;
   bool m_isPoolWorker;
   Mutex& m_mutexAlive;
   ThreadCompletionObserver* m_threadCompletionObserver;
   KeyValuePairs m_attributes;
   
   // disallow copying
   Thread(const Thread&);
   Thread& operator=(const Thread&);

};

}

#endif
