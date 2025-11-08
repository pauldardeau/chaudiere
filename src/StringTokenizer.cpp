// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <cstdio>
#include <cstring>
#include <stdexcept>

#include "StringTokenizer.h"
#include "BasicException.h"
#include "Logger.h"
#include "StrUtils.h"

static const std::string SPACE = " ";

using namespace chaudiere;

//******************************************************************************

StringTokenizer::StringTokenizer(const std::string& withTokens) :
   m_withTokens(withTokens),
   m_delimiter(SPACE),
   m_numberTokens(0),
   m_indexToken(0) {

   LOG_INSTANCE_CREATE("StringTokenizer")
   parse();
}

//******************************************************************************

StringTokenizer::StringTokenizer(const std::string& withTokens,
                                 const std::string& delimiter) :
   m_withTokens(withTokens),
   m_delimiter(delimiter),
   m_numberTokens(0),
   m_indexToken(0) {

   LOG_INSTANCE_CREATE("StringTokenizer")
   parse();
}

//******************************************************************************

StringTokenizer::~StringTokenizer() {
   LOG_INSTANCE_DESTROY("StringTokenizer")
}

//******************************************************************************

void StringTokenizer::parse() {
   m_tokens = StrUtils::split(m_withTokens, m_delimiter);
   m_numberTokens = m_tokens.size();
}

//******************************************************************************

bool StringTokenizer::hasMoreTokens() const {
   return (m_indexToken < m_numberTokens);
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

const std::vector<std::string>& StringTokenizer::getTokens() const {
   return m_tokens;
}

//******************************************************************************

