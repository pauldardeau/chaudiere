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
   ThreadPoolDispatch();

   ThreadPoolDispatch(const ThreadPoolDispatch&) = delete;
   ThreadPoolDispatch& operator=(const ThreadPoolDispatch&) = delete;
   
   /**
    * Destructor
    */
   ~ThreadPoolDispatch();
   
   // ThreadPoolDispatcher
   /**
    *
    * @return
    */
   virtual bool start();
   
   /**
    *
    * @return
    */
   virtual bool stop();
   
   /**
    *
    * @param runnableRequest
    * @return
    * @see Runnable()
    */
   virtual bool addRequest(Runnable* runnableRequest);


   
private:
   bool m_isRunning;
};

}

#endif
