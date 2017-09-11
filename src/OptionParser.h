// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_OPTIONPARSER_H
#define CHAUDIERE_OPTIONPARSER_H

#include <string>

#include "KeyValuePairs.h"


namespace chaudiere
{

   /**
    * OptionParser is a utility class for parsing command-line options. It's
    * inspired by Python's OptionParser in optparse module.
    */
   class OptionParser
   {
   private:
      KeyValuePairs m_kvpBooleanDefs;
      KeyValuePairs m_kvpStringDefs;
      KeyValuePairs m_kvpBooleanValues;
      KeyValuePairs m_kvpStringValues;


   public:
      /**
       * Default constructor
       */
      OptionParser();
      
      /**
       * Copy constructor
       * @param copy the source of the copy
       */
      OptionParser(const OptionParser& copy);
      
      /**
       * Destructor
       */
      ~OptionParser() {}
   
      /**
       * Copy operator
       * @param copy the source of the copy
       * @return reference to the target of the copy
       */
      OptionParser& operator=(const OptionParser& copy);
      
      /**
       * Adds an option that simply flags a feature/capability as present/enabled or not
       * @param option the name of option as specified on command line
       * @param destVariable the destination to populate if the option is present
       * @return boolean indicating whether option was added
       */
      bool addBooleanOption(const std::string& option,
                            const std::string& destVariable);
      
      /**
       * Adds a string option that may be present on the command line
       * @param option the name of the option as specified on the command line
       * @param destVariable the destination to populate if the option is present
       * @return boolean indicating whether option was added
       */
      bool addOption(const std::string& option,
                     const std::string& destVariable);
   
      /**
       * Determines if the specified string option is present
       * @param option the name of the option (as specified on the command line)
       * @return boolean indicating whether the option is present
       */
      bool hasOption(const std::string& option) const;
      
      /**
       * Retrieves the value associated with the specified string option
       * @param option the name of the option whose value is desired
       * @return value for the corresponding option name
       * @throw InvalidKeyException
       * @see hasOption
       */
      const std::string& getOptionValue(const std::string& option) const;
   
      /**
       * Determines if the specified boolean option is present
       * @param option the name of the option (as specified on the command line)
       * @return boolean indicating if the option is present
       */
      bool hasBooleanOption(const std::string& option) const;

      /**
       * Parses the command line arguments
       * @param argc number of arguments
       * @param argv command line arguments
       */
      void parseArgs(int argc, const char* argv[]);
   };

}

#endif
