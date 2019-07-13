// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_RUNNABLE_H
#define CHAUDIERE_RUNNABLE_H

#include <string>
#include <memory>

#include "RunCompletionObserver.h"
#include "Logger.h"

namespace chaudiere
{

/**
 * Runnable is an abstract base class that's conceptually very similar to
 * Java's Runnable interface for classes that can be used in background
 * threads.
 */
class Runnable
{
public:
   /**
    * Default constructor
    */
   Runnable() :
      m_completionObserver(NULL),
      m_runByThreadId(0),
      m_autoDelete(false) {
   }

   /**
    * Destructor
    */
   virtual ~Runnable() {}
    

   /**
    *
    */
   virtual void run() = 0;
   
   /**
    *
    * @param runByThreadId
    */
   virtual void setRunByThreadId(int runByThreadId) {
      m_runByThreadId = runByThreadId;
   }
   
   /**
    *
    * @return
    */
   virtual int getRunByThreadId() const {
      return m_runByThreadId;
   }
   
   /**
    *
    * @param runByThreadWorkerId
    */
   virtual void setRunByThreadWorkerId(const std::string& runByThreadWorkerId) {
      m_runByThreadWorkerId = runByThreadWorkerId;
   }
   
   /**
    *
    * @return
    */
   virtual const std::string& getRunByThreadWorkerId() const {
      return m_runByThreadWorkerId;
   }
   
   /**
    *
    * @param completionObserver
    * @see RunCompletionObserver()
    */
   virtual void setCompletionObserver(RunCompletionObserver* completionObserver) {
       m_completionObserver = completionObserver;
   }
   
   /**
    * This should only be called AFTER the run method has completed
    */
   virtual void notifyOnCompletion() {
      if (m_completionObserver != NULL) {
         m_completionObserver->notifyRunComplete(this);
      }
   }

   virtual bool isAutoDelete() const {
      return m_autoDelete;
   }

   virtual void setAutoDelete() {
      m_autoDelete = true;
   }
   
private:
   RunCompletionObserver* m_completionObserver;
   std::string m_runByThreadWorkerId;
   int m_runByThreadId;
   bool m_autoDelete;

   // disallow copies
   Runnable(const Runnable&);
   Runnable& operator=(const Runnable&);
};

}

#endif
