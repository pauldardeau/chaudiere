// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <sstream>
#include <stdexcept>
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
   int decimals_found = 0;
   const int s_length = s.length();
   for (int i = 0; i < s_length; ++i) {
      const char c = s[i];
      if (c == '-') {
         if (i > 0) {
            return false;
         }
      } else if (allow_decimal && (c == '.')) {
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
   if (!OnlyIntegerDigits(s)) {
      throw NumberFormatException(s);
   }

   const int intValue = ::atoi(s.c_str());
   if (intValue == 0) {
      if (s != ZERO) {
         throw NumberFormatException(s);
      }
   }

   return intValue;
}

//******************************************************************************

long StrUtils::parseLong(const std::string& s) {
   if (!OnlyIntegerDigits(s)) {
      throw NumberFormatException(s);
   }

   const long longValue = ::atol(s.c_str());
   if (longValue == 0) {
      if (s != ZERO) {
         throw NumberFormatException(s);
      }
   }

   return longValue;
}

//******************************************************************************

float StrUtils::parseFloat(const std::string& s) {
   if (!OnlyIntegerDigits(s, true)) {
      throw NumberFormatException(s);
   }

   const float floatValue = ::atof(s.c_str());
   if (floatValue == 0) {
      if (s != ZERO) {
         throw NumberFormatException(s);
      }
   }

   return floatValue;
}

//******************************************************************************

double StrUtils::parseDouble(const std::string& s) {
   if (!OnlyIntegerDigits(s, true)) {
      throw NumberFormatException(s);
   }

   const double doubleValue = ::atof(s.c_str());
   if (doubleValue == 0) {
      if (s != ZERO) {
         throw NumberFormatException(s);
      }
   }

   return doubleValue;
}

//******************************************************************************

std::string StrUtils::toString(int i) {
   char buffer[40];
   ::memset(buffer, 0, sizeof(buffer));
   ::snprintf(buffer, sizeof(buffer), "%d", i);
   return std::string(buffer);
}

//******************************************************************************

std::string StrUtils::toString(long l) {
   char buffer[40];
   ::memset(buffer, 0, sizeof(buffer));
   ::snprintf(buffer, sizeof(buffer), "%ld", l);
   return std::string(buffer);
}

//******************************************************************************

std::string StrUtils::toString(unsigned long l) {
   char buffer[40];
   ::memset(buffer, 0, sizeof(buffer));
   ::snprintf(buffer, sizeof(buffer), "%lu", l);
   return std::string(buffer);
}

//******************************************************************************

std::string StrUtils::toString(float f) {
   char buffer[40];
   ::memset(buffer, 0, sizeof(buffer));
   ::snprintf(buffer, sizeof(buffer), "%f", f);
   return std::string(buffer);
}

//******************************************************************************

std::string StrUtils::toString(double d) {
   char buffer[40];
   ::memset(buffer, 0, sizeof(buffer));
   ::snprintf(buffer, sizeof(buffer), "%f", d);
   return std::string(buffer);
}

//******************************************************************************

std::string StrUtils::charToString(char c) {
   char buffer[2];
   buffer[0] = c;
   buffer[1] = '\0';
   return std::string(buffer);
}

//******************************************************************************

void StrUtils::toLowerCase(std::string& s) {
   if (!s.empty()) {
      for (std::size_t i = 0; i < s.length(); i++) {
         s[i] = tolower(s[i]);
      }
   }
}

//******************************************************************************

void StrUtils::toUpperCase(std::string& s) {
   if (!s.empty()) {
      for (std::size_t i = 0; i < s.length(); i++) {
         s[i] = toupper(s[i]);
      }
   }
}

//******************************************************************************

bool StrUtils::startsWith(const std::string& haystack,
                          const std::string& needle) {
   if (!haystack.empty() && !needle.empty()) {
      const std::string::size_type haystackLength = haystack.length();
      const std::string::size_type needleLength = needle.length();
        
      if (haystackLength >= needleLength) {
         for (std::string::size_type i = 0; i < needleLength; ++i) {
            if (haystack[i] != needle[i]) {
               return false;
            }
         }
            
         return true;
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
         unsigned long int haystackIndex = haystackLength - 1;
         
         for (long i = needleLength-1; i >= 0; --i) {
            if (haystack[haystackIndex--] != needle[i]) {
               return false;
            }
         }
            
         return true;
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
    
   int newLength = (int) stringLen - 1;   // start at last character before nullptr
    
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
   if (s.empty()) {
      return s;
   }
   
   const std::string::size_type stringLen = s.length();
   std::string::size_type leadingStripChars = 0;
   
   while ((leadingStripChars < stringLen) &&
          (s[leadingStripChars] == stripChar)) {
      ++leadingStripChars;
   }
    
   // Any leading characters to strip?
   if (leadingStripChars > 0) {
      s = s.substr(leadingStripChars);
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
    
   std::string::size_type leadingChars = 0;
    
   for (std::string::size_type i = 0; i < len; ++i) {
      if (s[i] == strip) {
         ++leadingChars;
      } else {
         break;
      }
   }
    
   if (leadingChars == len) {
      return std::string(EMPTY);
   }
    
   int trailingChars = 0;
    
   for (int i = (int) len - 1; i >= 0; --i) {
      if (s[i] == strip) {
         ++trailingChars;
      } else {
         break;
      }
   }
    
   return s.substr(leadingChars, len - trailingChars - leadingChars);
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
   bool parsing = true;
   size_t pos_current = 0;
   size_t pos_delimiter;
   std::vector<std::string> tokens;
   const size_t s_length = s.length();
   const size_t delim_length = delim.length();

   while (parsing) {
      pos_delimiter = s.find(delim, pos_current);
      if (pos_delimiter == std::string::npos) {
         int num_chars = s_length - pos_current;
         if (num_chars > 0) {
            tokens.push_back(s.substr(pos_current, num_chars));
         }
         parsing = false;
      } else {
         int num_chars = pos_delimiter - pos_current;
         if (num_chars > 0) {
            tokens.push_back(s.substr(pos_current, num_chars));
            pos_current += num_chars;
         }
         pos_current += delim_length;
      }
   }

   return tokens;
}

//******************************************************************************
