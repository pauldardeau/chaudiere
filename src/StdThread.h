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
   StdThread() noexcept;

   explicit StdThread(const std::string& name) noexcept;
   
   /**
    *
    * @param runnable
    * @see Runnable()
    */
   explicit StdThread(Runnable* runnable) noexcept;

   explicit StdThread(Runnable* runnable, const std::string& name) noexcept;
   
   /**
    * Destructor
    */
   virtual ~StdThread() noexcept;
   
   /**
    *
    * @return
    */
   virtual bool start() noexcept override;
   
   /**
    *
    */
   virtual void run() override;
   
   /**
    *
    * @param thread
    */
   void runThread(StdThread* thread) noexcept;
   
   /**
    *
    * @return
    */
   std::thread::native_handle_type getHandle() noexcept;
   
   /**
    *
    * @return
    */
   std::thread::id getStdThreadId() const noexcept;

   const std::string& getName() const noexcept;
   
   
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
