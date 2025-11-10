// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_OPTIONPARSER_H
#define CHAUDIERE_OPTIONPARSER_H

#include <string>
#include <set>

#include "KeyValuePairs.h"


namespace chaudiere
{

   /**
    * OptionParser is a utility class for parsing command-line options.
    */
   class OptionParser
   {
   private:
      std::set<std::string> m_flags;
      std::set<std::string> m_flagsPresent;
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
       * Adds an option that simply flags a feature/capability as
       * present/enabled or not
       * @param option the name of option as specified on command line
       * @return boolean indicating whether option was added
       */
      bool addFlagOption(const std::string& option);

      /**
       * Determines whether a flag name is defined
       * @param flag the name of the flag
       * @return boolean indicating whether flag is defined
       */
      bool acceptsFlag(const std::string& flag) const;

      /**
       * Adds a string option that may be present on the command line
       * @param option the name of the option as specified on the command line
       * @param defaultValue the default value to use if not specified
       * @return boolean indicating whether option was added
       */
      bool addOption(const std::string& option,
                     const std::string& defaultValue);

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
      bool hasFlag(const std::string& option) const;

      /**
       * Parses the command line arguments
       * @param argc number of arguments
       * @param argv command line arguments
       */
      void parseArgs(int argc, const char* argv[]);
   };

}

#endif
