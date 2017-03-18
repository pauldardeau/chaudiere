// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_STRUTILS_H
#define CHAUDIERE_STRUTILS_H

#include <string>

namespace chaudiere
{

/**
 * StrUtils is a utility class of convenience methods for working with strings.
 */
class StrUtils
{
public:
   /**
    * Parse a string for integer value
    * @param s the string to parse for integer
    * @return integer value
    */
   static int parseInt(const std::string& s);

   /**
    * Parse a string for long value
    * @param s the string to parse for long
    * @return long value
    */
   static long parseLong(const std::string& s);

   /**
    * Parse a string for float value
    * @param s the string to parse for float
    * @return float value
    */
   static float parseFloat(const std::string& s);

   /**
    * Parse a string for double value
    * @param s the string to parse for double
    * @return double value
    */
   static double parseDouble(const std::string& s);
   
   /**
    * Construct a string from an integer
    * @param i integer value to convert to string
    * @return string version of integer value
    */
   static std::string toString(int i);

   /**
    * Construct a string from a long
    * @param l long value to convert to string
    * @return string version of long value
    */
   static std::string toString(long l);

   
   /**
    *
    * @param s
    * @return
    */
   static std::string strip(const std::string& s);
   
  /**
   *
   * @param s
   * @param strip
   * @return
   */
   static std::string strip(const std::string& s, char strip);
    
   /**
    *
    * @param haystack
    * @param needle
    * @return
    */
   static bool startsWith(const std::string& haystack,
                          const std::string& needle);
                          
   /**
    *
    * @param haystack
    * @param needle
    * @return
    */
   static bool endsWith(const std::string& haystack,
                        const std::string& needle);

   /**
    *
    * @param haystack
    * @param needle
    * @return
    */
   static bool containsString(const std::string& haystack,
                              const std::string& needle);

   /**
    *
    * @param s
    */
   static void toUpperCase(std::string& s);
   
   /**
    *
    * @param s
    */
   static void toLowerCase(std::string& s);

   /**
    *
    * @param s
    * @param searchFor
    * @param replaceWith
    * @return
    */
   static std::string& replaceAll(std::string& s,
                                  const std::string& searchFor,
                                  const std::string& replaceWith);

   /**
    *
    * @param s
    * @param strip
    * @return
    */
   static std::string& strip(std::string& s, char strip=' ');
   
   /**
    *
    * @param s
    * @param strip
    */
   static std::string& stripTrailing(std::string& s, char strip);
   
   /**
    *
    * @param s
    * @param strip
    * @return
    */
   static std::string& stripLeading(std::string& s, char strip);
   
   /**
    *
    * @param s
    * @return
    */
   static std::string& trimLeadingSpaces(std::string& s);
   
   /**
    *
    * @param s
    * @return
    */
   static std::string trim(const std::string& s);
   
   /**
    *
    * @param s
    * @param padChar
    * @param paddedLength
    */
   static void padRight(std::string& s,
                        char padChar,
                        std::string::size_type paddedLength);

   /**
    *
    * @param s
    * @param padChar
    * @param paddedLength
    */
   static void padLeft(std::string& s,
                       char padChar,
                       std::string::size_type paddedLength);

   /**
    *
    * @param s
    * @return
    */
   static std::string gzipCompress(const std::string& s);

   /**
    *
    * @param s
    * @return
    */
   static std::string gzipDecompress(const std::string& s);
   
};

}

#endif
