// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_THREADPOOLDISPATCH_H
#define CHAUDIERE_THREADPOOLDISPATCH_H

#include "ThreadPoolDispatcher.h"


namespace chaudiere
{
   class Runnable;

/**
 *
 */
class ThreadPoolDispatch : public ThreadPoolDispatcher
{
public:
   /**
    * Default constructor
    */
   ThreadPoolDispatch() noexcept;
   
   /**
    * Destructor
    */
   ~ThreadPoolDispatch() noexcept;
   
   // ThreadPoolDispatcher
   /**
    *
    * @return
    */
   virtual bool start() noexcept override;
   
   /**
    *
    * @return
    */
   virtual bool stop() noexcept override;
   
   /**
    *
    * @param runnableRequest
    * @return
    * @see Runnable()
    */
   virtual bool addRequest(std::shared_ptr<Runnable> runnableRequest) noexcept override;

   // disallow copies
   ThreadPoolDispatch(const ThreadPoolDispatch&) = delete;
   ThreadPoolDispatch(ThreadPoolDispatch&&) = delete;
   ThreadPoolDispatch& operator=(const ThreadPoolDispatch&) = delete;
   ThreadPoolDispatch& operator=(ThreadPoolDispatch&&) = delete;

   
private:
   bool m_isRunning;
   
};

}

#endif
