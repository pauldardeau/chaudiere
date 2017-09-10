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
   m_kvpBooleanDefs(copy.m_kvpBooleanDefs),
   m_kvpStringDefs(copy.m_kvpStringDefs),
   m_kvpBooleanValues(copy.m_kvpBooleanValues),
   m_kvpStringValues(copy.m_kvpStringValues) {
}

//******************************************************************************

OptionParser& OptionParser::operator=(const OptionParser& copy) {
   if (this == &copy) {
      return *this;
   }
   
   m_kvpBooleanDefs = copy.m_kvpBooleanDefs;
   m_kvpStringDefs = copy.m_kvpStringDefs;
   m_kvpBooleanValues = copy.m_kvpBooleanValues;
   m_kvpStringValues = copy.m_kvpStringValues;
   
   return *this;
}

//******************************************************************************

bool OptionParser::addBooleanOption(const std::string& option,
                                    const std::string& destVariable) {
   if (!option.empty() && !destVariable.empty()) {
      m_kvpBooleanDefs.addPair(option, destVariable);
      return true;
   }
   return false;
}

//******************************************************************************

bool OptionParser::addOption(const std::string& option,
                             const std::string& destVariable) {
   if (!option.empty() && !destVariable.empty()) {
      m_kvpStringDefs.addPair(option, destVariable);
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

bool OptionParser::hasBooleanOption(const std::string& option) const {
   return m_kvpBooleanValues.hasKey(option);
}

//******************************************************************************

void OptionParser::parseArgs(int* argc, char** argv[]) {
   // first argument is program name, skip over it
   --(*argc);
   ++(*argv);
   
   bool parsing = true;
   
   while (parsing) {
      if (*argc > 0) {
         std::string arg = (*argv)[0];
         
         // is it a boolean option?
         if (m_kvpBooleanDefs.hasKey(arg)) {
            const std::string& destVar = m_kvpBooleanDefs.getValue(arg);
            m_kvpBooleanValues.addPair(destVar, "true");
            --(*argc);
            ++(*argv);
         } else if (m_kvpStringDefs.hasKey(arg)) {
            if (*argc > 1) {
               std::string argValue = (*argv)[1];
               const std::string& destVar = m_kvpStringDefs.getValue(arg);
               m_kvpStringValues.addPair(destVar, argValue);
               (*argc) -= 2;
               ++(*argv);
               ++(*argv);
            } else {
               parsing = false;
            }
         } else {
            parsing = false;
         }
      } else {
         parsing = false;
      }
   }
}

//******************************************************************************
