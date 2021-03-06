// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <sstream>

#include "StdThread.h"
#include "BasicException.h"
#include "Logger.h"

using namespace chaudiere;

//******************************************************************************
//******************************************************************************

void StdThread::runThread(StdThread* thread)
{
   std::stringstream ss;
   ss << std::this_thread::get_id();
   const std::string threadIdString = ss.str();
   
   thread->setThreadId(threadIdString);
   
   //unsigned long rc = 0L;
   
   try {
      thread->setAlive(true);
      
      // if we had a runnable passed in on the constructor, use it.  otherwise,
      // call "run" on the thread object itself.
      
      Runnable* runnable = thread->getRunnable();
      
      if (runnable) {
         runnable->run();
      } else {
         thread->run();
      }
   } catch (...) {
      //rc = 1L;
      LOG_ERROR("StdThread::runThread exception caught running thread")
   }
   
   thread->setAlive(false);
   thread->notifyOnCompletion();
}

//******************************************************************************
//******************************************************************************

StdThread::StdThread() :
   StdThread(NULL, "") {
   LOG_INSTANCE_CREATE("StdThread")
}

StdThread::StdThread(const std::string& name) :
   StdThread(NULL, name) {
   LOG_INSTANCE_CREATE("StdThread")
}

//******************************************************************************

StdThread::StdThread(Runnable* runnable) :
   StdThread(NULL, "") {
   LOG_INSTANCE_CREATE("StdThread")
}

StdThread::StdThread(Runnable* runnable, const std::string& name) :
   Thread(&m_mutexAlive, runnable),
   m_name(name) {
   LOG_INSTANCE_CREATE("StdThread")
}

//******************************************************************************

StdThread::~StdThread() {
   LOG_INSTANCE_DESTROY("StdThread")
}

//******************************************************************************

bool StdThread::start() {
   bool isSuccess = false;
   
   //TODO: research and fix!
   // when the following line is uncommented, the following compiler error
   // is produced:
   // "Attempt to use a deleted function".
   //m_thread = std::thread(&StdThread::runThread, this);
   
   return isSuccess;
}

//******************************************************************************

void StdThread::run() {
   // This method should never be called.  If you've subclassed Thread, then
   // you need to implement "void run()" in your derived class.
   throw BasicException("this method should not be called");
}

//******************************************************************************

std::thread::native_handle_type StdThread::getHandle() {
   return m_thread.native_handle();
}

//******************************************************************************

std::thread::id StdThread::getStdThreadId() const {
   return m_thread.get_id();
}

//******************************************************************************

const std::string& StdThread::getName() const {
   return m_name;
}

//******************************************************************************
