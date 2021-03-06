// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <cstdio>
#include <cstring>
#include <stdexcept>

#include "StringTokenizer.h"
#include "BasicException.h"
#include "Logger.h"

static const std::string SPACE = " ";

using namespace chaudiere;

//******************************************************************************

StringTokenizer::StringTokenizer(const std::string& withTokens) :
   m_withTokens(withTokens),
   m_delimiter(SPACE),
   m_posTokens(m_withTokens.c_str()),
   m_posDelimiter(m_delimiter.c_str()),
   m_posCurrent(0),
   m_stringLength(m_withTokens.length()),
   m_isConstructing(true),
   m_numberTokens(0),
   m_indexToken(0) {

   LOG_INSTANCE_CREATE("StringTokenizer")
   parse();
   m_isConstructing = false;
}

//******************************************************************************

StringTokenizer::StringTokenizer(const std::string& withTokens,
                                 const std::string& delimiter) :
   m_withTokens(withTokens),
   m_delimiter(delimiter),
   m_posTokens(m_withTokens.c_str()),
   m_posDelimiter(m_delimiter.c_str()),
   m_posCurrent(0),
   m_stringLength(m_withTokens.length()),
   m_isConstructing(true),
   m_numberTokens(0),
   m_indexToken(0) {

   LOG_INSTANCE_CREATE("StringTokenizer")
   parse();
   m_isConstructing = false;
}

//******************************************************************************

StringTokenizer::~StringTokenizer() {
   LOG_INSTANCE_DESTROY("StringTokenizer")
}

//******************************************************************************

void StringTokenizer::parse() {
   if (m_withTokens.empty()) {
      m_posCurrent = std::string::npos;
   } else {
      m_posCurrent = ::strspn(m_posTokens, m_posDelimiter);

      while (m_posCurrent != std::string::npos) { // while (hasMoreTokens()) {
         m_tokens.push_back(extractNextToken());
      }

      m_numberTokens = m_tokens.size();
   }
}

//******************************************************************************

bool StringTokenizer::hasMoreTokens() const {
   if (m_isConstructing) {
      return (m_posCurrent != std::string::npos);
   } else {
      return (m_indexToken < m_numberTokens);
   }
}

//******************************************************************************

std::size_t StringTokenizer::countTokens() const {
   return m_numberTokens;
}

//******************************************************************************

const std::string& StringTokenizer::nextToken() {
   if (m_indexToken < m_numberTokens) {
      return m_tokens[m_indexToken++];
   } else {
      throw std::out_of_range("no more tokens");
   }
}

//******************************************************************************

std::string StringTokenizer::extractNextToken() {
   if (m_posCurrent == std::string::npos) {
      throw std::out_of_range("no more tokens");
   }
   
   const char* posWithTokens = m_posTokens + m_posCurrent;
   
   const size_t numNonDelimiterChars = ::strcspn(posWithTokens, m_posDelimiter);
   
   if (numNonDelimiterChars > 0) {
      std::size_t posStart = m_posCurrent;
      m_posCurrent += numNonDelimiterChars;
      posWithTokens += numNonDelimiterChars;
      
      m_posCurrent += ::strspn(posWithTokens, m_posDelimiter);
      
      if (m_posCurrent >= m_stringLength) {
         m_posCurrent = std::string::npos;
      }
      
      return m_withTokens.substr(posStart, numNonDelimiterChars);
   } else {
      // we're on the last token
      const std::string::size_type posLastToken = m_posCurrent;
      m_posCurrent = std::string::npos;
      
      if (m_posCurrent >= m_stringLength) {
         m_posCurrent = std::string::npos;
      }
      
      return m_withTokens.substr(posLastToken);
   }
}

//******************************************************************************

const std::vector<std::string>& StringTokenizer::getTokens() const {
   return m_tokens;
}
