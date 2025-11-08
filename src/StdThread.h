// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_STDTHREAD_H
#define CHAUDIERE_STDTHREAD_H

#include <string>
#include <thread>

#include "Thread.h"
#include "StdMutex.h"

namespace chaudiere
{

/**
 * StdThread is a Thread that's implemented using std::thread (C++ 11).
 */
class StdThread : public Thread
{
public:
   /**
    * Default constructor
    */
   StdThread();

   explicit StdThread(const std::string& name);
   
   /**
    * Constructs a c++11 thread using the specified Runnable
    * @param runnable the executable object that the thread will run
    * @see Runnable()
    */
   explicit StdThread(Runnable* runnable);

   explicit StdThread(Runnable* runnable, const std::string& name);
   
   /**
    * Destructor
    */
   virtual ~StdThread();
   
   /**
    *
    * @return
    */
   virtual bool start();
   
   /**
    *
    */
   virtual void run();
   
   /**
    *
    * @param thread
    */
   void runThread(StdThread* thread);
   
   /**
    *
    * @return
    */
   std::thread::native_handle_type getHandle();
   
   /**
    *
    * @return
    */
   std::thread::id getStdThreadId() const;

   const std::string& getName() const;

   virtual void join();
   
   
private:
   // disallow copying
   StdThread(const StdThread&);
   StdThread& operator=(const StdThread&);

   std::thread m_thread;
   StdMutex m_mutexAlive;
   std::string m_name;
   
};

}

#endif
