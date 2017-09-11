// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "OptionParser.h"
#include "InvalidKeyException.h"

using namespace chaudiere;

//******************************************************************************

OptionParser::OptionParser() {
}

//******************************************************************************

OptionParser::OptionParser(const OptionParser& copy) :
   m_flags(copy.m_flags),
   m_flagsPresent(copy.m_flagsPresent),
   m_kvpStringValues(copy.m_kvpStringValues) {
}

//******************************************************************************

OptionParser& OptionParser::operator=(const OptionParser& copy) {
   if (this == &copy) {
      return *this;
   }
   
   m_flags = copy.m_flags;
   m_flagsPresent = copy.m_flagsPresent;
   m_kvpStringValues = copy.m_kvpStringValues;
   
   return *this;
}

//******************************************************************************

bool OptionParser::addFlagOption(const std::string& option) {
   if (!option.empty()) {
      m_flags.insert(option);
      return true;
   }
   return false;
}

//******************************************************************************

bool OptionParser::addOption(const std::string& option,
                             const std::string& defaultValue) {
   if (!option.empty()) {
      m_kvpStringValues.addPair(option, defaultValue);
      return true;
   }
   return false;
}

//******************************************************************************

bool OptionParser::hasOption(const std::string& option) const {
   return m_kvpStringValues.hasKey(option);
}

//******************************************************************************

const std::string& OptionParser::getOptionValue(const std::string& option) const {
   if (m_kvpStringValues.hasKey(option)) {
      return m_kvpStringValues.getValue(option);
   }
   
   throw InvalidKeyException(option);
}

//******************************************************************************

bool OptionParser::hasFlag(const std::string& option) const {
   return m_flagsPresent.find(option) != m_flagsPresent.end();
}

//******************************************************************************

bool OptionParser::acceptsFlag(const std::string& flag) const {
   return m_flags.find(flag) != m_flags.end();
}

//*****************************************************************************

void OptionParser::parseArgs(int argc, const char* argv[]) {
   // first argument is program name, skip over it
   --argc;
   ++argv;
   
   bool parsing = true;
   
   while (parsing) {
      if (argc > 0) {
         std::string arg = argv[0];
         
         // is it a boolean option?
         if (acceptsFlag(arg)) {
            m_flagsPresent.insert(arg);
            --argc;
            ++argv;
         } else {
            if (argc > 1) {
               std::string argValue = argv[1];
               m_kvpStringValues.addPair(arg, argValue);
               argc -= 2;
               ++argv;
               ++argv;
            } else {
               parsing = false;
            }
         }
      } else {
         parsing = false;
      }
   }
}

//******************************************************************************
