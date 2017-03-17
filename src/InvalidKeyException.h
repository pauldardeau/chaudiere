// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_INVALIDKEYEXCEPTION_H
#define CHAUDIERE_INVALIDKEYEXCEPTION_H

#include "BasicException.h"

namespace chaudiere
{

/**
 * InvalidKeyException is an exception that indicates that the specified
 * key is invalid or not found.
 */
class InvalidKeyException : public BasicException
{
public:
   /**
    * Constructs an InvalidKeyException with the specified key
    * @param key the name of the invalid key
    */
   explicit InvalidKeyException(const std::string& key);
   
   /**
    * Copy constructor
    * @param copy the source of the copy
    */
   InvalidKeyException(const InvalidKeyException& copy);
   
   /**
    * Destructor
    */
   virtual ~InvalidKeyException() throw ();

   /**
    * Retrieves the class name (type) of the exception
    * @return class name
    */
   virtual const char* getType() const {
      return "InvalidKeyException";
   }
   
   /**
    * Copy operator
    * @param copy the source of the copy
    * @return reference to the updated reference
    */
   InvalidKeyException& operator=(const InvalidKeyException& copy);
   
   /**
    * Retrieves the name of the invalid key
    * @return name of the invalid key
    */
   const std::string& getKey() const;
   
private:
   std::string m_key;
};

}

#endif
