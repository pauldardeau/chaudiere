// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_STDCONDITIONVARIABLE_H
#define CHAUDIERE_STDCONDITIONVARIABLE_H

#include <string>
#include <condition_variable>

#include "ConditionVariable.h"

namespace chaudiere
{

/**
 *
 */
class StdConditionVariable : public ConditionVariable
{
public:
   /**
    * Default constructor
    */
   StdConditionVariable();

   StdConditionVariable(const std::string& name);
   
   /**
    * Destructor
    */
   ~StdConditionVariable();
   
   /**
    *
    * @param mutex
    * @return
    * @see Mutex()
    */
   virtual bool wait(Mutex* mutex);
   
   /**
    *
    */
   virtual void notifyOne();
   
   /**
    *
    */
   virtual void notifyAll();

   const std::string& getName() const;
   

private:
   // disallow copies
   StdConditionVariable(const StdConditionVariable&);
   StdConditionVariable& operator=(const StdConditionVariable&);

   std::condition_variable m_cond;
   std::string m_name;
};

}

#endif
