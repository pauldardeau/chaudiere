// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_THREADPOOLDISPATCHER_H
#define CHAUDIERE_THREADPOOLDISPATCHER_H

#include <memory>



namespace chaudiere
{
   class Runnable;

/**
 * ThreadPoolDispatcher is an abstract base class for handing off requests
 * to a thread pool to be dispatched.
 */
class ThreadPoolDispatcher
{
public:
   /**
    * Default constructor
    */
   ThreadPoolDispatcher() {}

   ThreadPoolDispatcher(const ThreadPoolDispatcher&) = delete;
   ThreadPoolDispatcher& operator=(const ThreadPoolDispatcher&) = delete;
   
   /**
    * Destructor
    */
   virtual ~ThreadPoolDispatcher() {}
   
   /**
    *
    * @return
    */
   virtual bool start() = 0;
   
   /**
    *
    * @return
    */
   virtual bool stop() = 0;
   
   /**
    *
    * @param runnableRequest
    * @return
    * @see Runnable()
    */
   virtual bool addRequest(Runnable* runnableRequest) = 0;
   
};

}

#endif
