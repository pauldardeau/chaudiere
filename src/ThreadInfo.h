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
   explicit ThreadInfo(const std::string& theThreadId) :
      threadId(theThreadId),
      isBusy(false) {
   }
   
   /**
    * Copy constructor
    * @param copy the source of the copy
    */
   ThreadInfo(const ThreadInfo& copy) :
      threadId(copy.threadId),
      isBusy(copy.isBusy) {
   }

   /**
    * Move constructor
    * @param move the source of the move
    */
   ThreadInfo(const ThreadInfo&& move) :
      threadId(std::move(move.threadId)),
      isBusy(move.isBusy) {
   }
   
   /**
    * Destructor
    */
   ~ThreadInfo() {
   }
   
   /**
    * Copy operator
    * @param copy the source of the copy
    * @return reference to the target of the copy
    */
   ThreadInfo& operator=(const ThreadInfo& copy) {
      if (this == &copy) {
         return *this;
      }
      
      threadId = copy.threadId;
      isBusy = copy.isBusy;
      
      return *this;
   }

   /**
    * Assignment move operator
    * @param move the source of the move
    * @return reference to the target of the move
    */
   ThreadInfo& operator=(const ThreadInfo&& move) {
      threadId = std::move(move.threadId);
      isBusy = move.isBusy;
      return *this;
   }

};

}

#endif
