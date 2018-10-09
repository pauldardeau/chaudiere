// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_NUMBERFORMATEXCEPTION_H
#define CHAUDIERE_NUMBERFORMATEXCEPTION_H

#include "BasicException.h"

namespace chaudiere
{

/**
 * NumberFormatException is an exception that indicates that a format
 * meant for numbers is invalid.
 */
class NumberFormatException : public BasicException
{
public:
   /**
    * Constructs a NumberFormatException with no detail message
    */
   NumberFormatException();

   /**
    * Constructs a NumberFormatException with the specified detail
    * @param s the detail message
    */
   explicit NumberFormatException(const std::string& s);
   
   /**
    * Copy constructor
    * @param copy the source of the copy
    */
   NumberFormatException(const NumberFormatException& copy);

   /**
    * Move constructor
    * @param move the source of the move
    */
   NumberFormatException(const NumberFormatException&& move);
   
   /**
    * Destructor
    */
   virtual ~NumberFormatException() throw ();

   /**
    * Retrieves the class name (type) of the exception
    * @return exception class name
    */
   virtual const char* getType() const {
      return "NumberFormatException";
   }
   
   /**
    * Assignment copy operator
    * @param copy the source of the copy
    * @return reference to the updated reference
    */
   NumberFormatException& operator=(const NumberFormatException& copy);

   /**
    * Assignment move operator
    * @param move the source of the move
    * @return reference to the updated reference
    */
   NumberFormatException& operator=(const NumberFormatException&& move);
   
};

}

#endif

