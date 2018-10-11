// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "StrUtils.h"
#include "NumberFormatException.h"
#include "StringTokenizer.h"

static const std::string EMPTY = "";
static const std::string SPACE = " ";
static const std::string ZERO = "0";

using namespace chaudiere;

//******************************************************************************

bool OnlyIntegerDigits(const std::string& s, bool allow_decimal=false) {
   // scan for valid characters (0-9)
   // '-' is allowed as first character only
   // if allow_decimal is true, '.' may only have single occurrence
   if (!allow_decimal) {
      return (std::string::npos == s.find_first_not_of("0123456789"));
   }

   int decimals_found = 0;
   const int s_length = s.length();
   for (int i = 0; i < s_length; ++i) {
      const char c = s[i];
      if (c == '-') {
         if (i > 0) {
            return false;
         }
      } else if (c == '.') {
         decimals_found++;
         if (decimals_found > 1) {
            return false;
         }
      } else {
         const int digit_value = c - '0';
         if (digit_value < 0 || digit_value > 9) {
            return false;
         }
      }
   }
   return true;
}

//******************************************************************************

int StrUtils::parseInt(const std::string& s) {
   if (std::string::npos != s.find_first_not_of("0123456789-")) {
      throw NumberFormatException(s);
   }

   try {
      return std::stoi(s);
   } catch (const std::invalid_argument& e) {
      throw NumberFormatException(s);
   }
}

//******************************************************************************

long StrUtils::parseLong(const std::string& s) {
   if (std::string::npos != s.find_first_not_of("0123456789-")) {
      throw NumberFormatException(s);
   }

   try {
      return std::stol(s);
   } catch (const std::invalid_argument& e) {
      throw NumberFormatException(s);
   }
}

//******************************************************************************

float StrUtils::parseFloat(const std::string& s) {
   if (!OnlyIntegerDigits(s, true)) {
      throw NumberFormatException(s);
   }

   try {
      return std::stof(s);
   } catch (const std::invalid_argument& e) {
      throw NumberFormatException(s);
   }
}

//******************************************************************************

double StrUtils::parseDouble(const std::string& s) {
   if (!OnlyIntegerDigits(s, true)) {
      throw NumberFormatException(s);
   }

   try {
      return std::stod(s);
   } catch (const std::invalid_argument& e) {
      throw NumberFormatException(s);
   }
}

//******************************************************************************

std::string StrUtils::toString(int i) {
   return std::to_string(i);
}

//******************************************************************************

std::string StrUtils::toString(long l) {
   return std::to_string(l);
}

//******************************************************************************

std::string StrUtils::toString(unsigned long l) {
   return std::to_string(l);
}

//******************************************************************************

std::string StrUtils::charToString(char c) {
   return std::string(1, c);
}

//******************************************************************************

void StrUtils::toLowerCase(std::string& s) {
   if (!s.empty()) {
      std::transform(s.begin(), s.end(), s.begin(), ::tolower);
   }
}

//******************************************************************************

void StrUtils::toUpperCase(std::string& s) {
   if (!s.empty()) {
      std::transform(s.begin(), s.end(), s.begin(), ::toupper);
   }
}

//******************************************************************************

bool StrUtils::startsWith(const std::string& haystack,
                          const std::string& needle) {
   // C++20 will have starts_with method
   if (!haystack.empty() && !needle.empty()) {
      const std::string::size_type needleLength = needle.length();
      if (haystack.length() >= needleLength) {
         return (0 == haystack.compare(0, needleLength, needle));
      }
   }
   
   return false;
}

//******************************************************************************

bool StrUtils::endsWith(const std::string& haystack,
                        const std::string& needle) {
   if (!haystack.empty() && !needle.empty()) {
      const std::string::size_type haystackLength = haystack.length();
      const std::string::size_type needleLength = needle.length();
        
      if (haystackLength >= needleLength) {
         return (0 == haystack.compare(haystackLength - needleLength,
                                       needleLength,
                                       needle));
      }
   }

   return false;
}

//******************************************************************************

bool StrUtils::containsString(const std::string& haystack,
                              const std::string& needle) {
   return (std::string::npos != haystack.find(needle));
}

//******************************************************************************

std::string& StrUtils::strip(std::string& s, char strip) {
   return stripLeading(stripTrailing(s, strip), strip);
}

//******************************************************************************

std::string& StrUtils::stripTrailing(std::string& s, char strip) {
   if (s.empty()) {
      return s;
   }
   
   const std::string::size_type stringLen = s.length();
    
   int newLength = (int) stringLen - 1;   // start at last character before NULL
    
   // Find out how many trailing characters we have
   while ((0 <= newLength) && (s[newLength] == strip)) {
      --newLength;
   }
    
   ++newLength;
   const std::string::size_type newLengthUnsigned = newLength;
    
   // Did we not have any characters to strip?
   if (newLengthUnsigned == stringLen) {
      return s;
   }
   
   s = s.substr(0, newLength);
   
   return s;
}

//******************************************************************************

std::string& StrUtils::stripLeading(std::string& s, char stripChar) {
   const std::string::size_type pos_non_matching = s.find_first_not_of(stripChar);
   if ((std::string::npos != pos_non_matching) && (pos_non_matching > 0)) {
      s.erase(0, pos_non_matching);
   }
   
   return s;
}

//******************************************************************************

std::string& StrUtils::trimLeadingSpaces(std::string& s) {
   const std::string::size_type posFirstNonSpace = s.find_first_not_of(SPACE);
   if ((std::string::npos != posFirstNonSpace) && (posFirstNonSpace > 0)) {
      s.erase(0, posFirstNonSpace);
   }
    
   return s;
}

//******************************************************************************

std::string StrUtils::trim(const std::string& s) {
   return StrUtils::strip(s);
}

//**************************************************************************

std::string StrUtils::strip(const std::string& s) {
   return StrUtils::strip(s, ' ');
}

//******************************************************************************

std::string StrUtils::strip(const std::string& s, char strip) {
   if (s.empty()) {
      return EMPTY;
   }
   
   const std::string::size_type len = s.length();
   std::string::size_type leadingChars = s.find_first_not_of(strip);
    
   if (leadingChars == len) {
      return EMPTY;
   }
    
   int trailingChars = 0;
    
   for (int i = (int) len - 1; i >= 0; --i) {
      if (s[i] == strip) {
         ++trailingChars;
      } else {
         break;
      }
   }
    
   return s.substr(leadingChars, len - trailingChars);
}

//******************************************************************************

std::string& StrUtils::replaceAll(std::string& s,
                                  const std::string& searchFor,
                                  const std::string& replaceWith) {
   std::string::size_type posSearchFor = s.find(searchFor);
    
   if (posSearchFor == std::string::npos) {
      return s;
   }
    
   const std::string::size_type lenSearchFor = searchFor.length();
   const std::string::size_type lenReplaceWith = replaceWith.length();
    
   do {
      s.replace(posSearchFor, lenSearchFor, replaceWith);
      posSearchFor = s.find(searchFor, posSearchFor + lenReplaceWith);
   } while (posSearchFor != std::string::npos);
    
   return s;
}

//******************************************************************************

void StrUtils::padRight(std::string& s, char padChar, std::string::size_type paddedLength) {
   if (s.length() < paddedLength) {
      s += std::string(paddedLength - s.length(), padChar);
   }
}

//******************************************************************************

void StrUtils::padLeft(std::string& s, char padChar, std::string::size_type paddedLength) {
   if (s.length() < paddedLength) {
      s.insert(0, std::string(paddedLength - s.length(), padChar));
   }
}

//******************************************************************************

std::string StrUtils::makeStringOfChar(char ch, int length) {
   if (length > 0) {
      return std::string(length, ch);
   } else {
      return EMPTY;
   }
}

//******************************************************************************

std::vector<std::string> StrUtils::split(const std::string& s,
                                         const std::string& delim) {
   std::vector<std::string> components;
   StringTokenizer st(s, delim);

   while (st.hasMoreTokens()) {
      components.push_back(st.nextToken());
   }

   return components;
}

//******************************************************************************

