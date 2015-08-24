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
   explicit Thread(Mutex& mutexAlive) noexcept;
   Thread(Mutex& mutexAlive, Runnable* runnable) noexcept;
   
   virtual ~Thread() noexcept;
   
   
   void setPoolWorkerStatus(bool isPoolWorker) noexcept;
   bool isPoolWorker() const noexcept;
   
   virtual bool start() noexcept = 0;
   virtual void run() override;
   
   bool isAlive() const noexcept;
   void setAlive(bool isAlive) noexcept;
   
   void registerThreadCompletionObserver(ThreadCompletionObserver* observer) noexcept;
   void clearThreadCompletionObserver() noexcept;
   void notifyOnCompletion() noexcept;
   
   
   Runnable* getRunnable() noexcept;
   
   
   void setAttribute(const std::string& key,
                     const std::string& value) noexcept;
   
   bool hasAttribute(const std::string& key) const noexcept;
   
   // throws InvalidKeyException
   const std::string& getAttribute(const std::string& key) const;
   
   void clearAttribute(const std::string& key) noexcept;
   
   void setThreadId(const std::string& threadId) noexcept;
   bool hasThreadId() const noexcept;
   const std::string& getThreadId() const;
   
   void setWorkerId(const std::string& workerId) noexcept;
   bool hasWorkerId() const noexcept;
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
