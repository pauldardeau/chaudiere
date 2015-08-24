// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_THREADINFO_H
#define CHAUDIERE_THREADINFO_H

#include <string>

namespace chaudiere
{

/**
 * ThreadInfo is used to track state of a Thread within a ThreadPool.
 */
class ThreadInfo
{
public:
   std::string threadId;
   bool isBusy;
   
   /**
    * Constructs a new ThreadInfo with the specified thread id
    * @param theThreadId the id for the thread
    */
   explicit ThreadInfo(const std::string& theThreadId) noexcept :
      threadId(theThreadId),
      isBusy(false)
   {
   }
   
   /**
    * Copy constructor
    * @param copy the source of the copy
    */
   ThreadInfo(const ThreadInfo& copy) noexcept :
      threadId(copy.threadId),
      isBusy(copy.isBusy)
   {
   }
   
   /**
    * Destructor
    */
   ~ThreadInfo() noexcept
   {
   }
   
   /**
    * Copy operator
    * @param copy the source of the copy
    * @return reference to the target of the copy
    */
   ThreadInfo& operator=(const ThreadInfo& copy) noexcept
   {
      if (this == &copy) {
         return *this;
      }
      
      threadId = copy.threadId;
      isBusy = copy.isBusy;
      
      return *this;
   }
   
};

}

#endif
