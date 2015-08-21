// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_THREADCOMPLETIONOBSERVER_H
#define CHAUDIERE_THREADCOMPLETIONOBSERVER_H


namespace chaudiere
{
   class Thread;

/**
 *
 */
class ThreadCompletionObserver
{
public:
   /**
    * Destructor
    */
   virtual ~ThreadCompletionObserver() noexcept {}
   
   /**
    *
    * @param thread
    * @see Thread()
    */
   virtual void notifyThreadComplete(Thread* thread) noexcept = 0;
};

}

#endif
