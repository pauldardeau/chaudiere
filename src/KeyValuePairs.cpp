// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "KeyValuePairs.h"
#include "InvalidKeyException.h"
#include "Logger.h"

using namespace std;
using namespace chaudiere;

//******************************************************************************

KeyValuePairs::KeyValuePairs() {
   LOG_INSTANCE_CREATE("KeyValuePairs")
}

//******************************************************************************

KeyValuePairs::KeyValuePairs(const KeyValuePairs& copy) :
   m_keyValues(copy.m_keyValues) {
   LOG_INSTANCE_CREATE("KeyValuePairs")
}

//******************************************************************************

KeyValuePairs::~KeyValuePairs() {
   LOG_INSTANCE_DESTROY("KeyValuePairs")
}

//******************************************************************************

KeyValuePairs& KeyValuePairs::operator=(const KeyValuePairs& copy) {
   if (&copy == this) {
      return *this;
   }
   
   m_keyValues = copy.m_keyValues;

   return *this;
}

//******************************************************************************

void KeyValuePairs::getKeys(std::vector<std::string>& keys) const {
   if (m_keyValues.empty()) {
      return;
   }

   keys.reserve(m_keyValues.size());
   const auto itEnd = m_keyValues.end();
   auto it = m_keyValues.begin();
   
   for ( ; it != itEnd; it++) {
      keys.push_back((*it).first);
   }
}

//******************************************************************************

bool KeyValuePairs::hasKey(const std::string& key) const {
   return (m_keyValues.find(key) != m_keyValues.end());
}

//******************************************************************************

const std::string& KeyValuePairs::getValue(const std::string& key) const {
   auto it = m_keyValues.find(key);
   if (it != m_keyValues.end()) {
      return (*it).second;
   } else {
      throw InvalidKeyException(key);
      return (*it).second;
   }
}

//******************************************************************************

void KeyValuePairs::addPair(const std::string& key,
                            const std::string& value) {
   m_keyValues[key] = value;
}

//******************************************************************************

bool KeyValuePairs::removePair(const std::string& key) {
   auto it = m_keyValues.find(key);
   if (it != m_keyValues.end()) {
      m_keyValues.erase(it);
      return true;
   }
   
   return false;
}

//******************************************************************************

void KeyValuePairs::clear() {
   m_keyValues.erase(m_keyValues.begin(), m_keyValues.end());
}

//******************************************************************************

std::size_t KeyValuePairs::size() const {
   return m_keyValues.size();
}

//******************************************************************************

bool KeyValuePairs::empty() const {
   return m_keyValues.empty();
}

//******************************************************************************

