// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_STRUTILS_H
#define CHAUDIERE_STRUTILS_H

#include <string>
#if __cplusplus >= 202002L
   // C++20 or later
   #include <string_view>
#endif
#include <vector>

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
    * Construct a string from an unsigned long
    * @param l unsigned long value to convert to string
    * @return string version of unsigned long value
    */
   static std::string toString(unsigned long l);

   /**
    * Construct a string from a long long
    * @param l long long value to convert to string
    * @return string version of long long value
    */
   static std::string toString(long long ll);

   /**
    * Construct a string from a std::size_t
    * @param s std::size_t value to convert to string
    * @return string version of std::size_t value
    */
   static std::string size_tToString(std::size_t s);

   /**
    * Construct a string from a float
    * @param f float value to convert to string
    * @return string version of float value
    */
   static std::string toString(float f);

   /**
    * Construct a string from a double
    * @param d double value to convert to string
    * @return string version of double value
    */
   static std::string toString(double d);

   /**
    * Construct a string from a single character
    * @param c the character to use as string
    * @return string verion of character value
    */
   static std::string charToString(char c);

   /**
    * Strip leading and trailing spaces from string
    * @param s the string to strip
    * @return stripped string
    */
   static std::string strip(const std::string& s);

  /**
   * Strip leading and trailing character 'strip' from string 's'
   * @param s the string to strip
   * @param strip the character to strip
   * @return stripped string
   */
   static std::string strip(const std::string& s, char strip);

   /**
    * Determines whether 'haystack' starts with 'needle'
    * @param haystack the string to search
    * @param needle the substring to search for
    * @return boolean indicating whether test is true
    */
   static bool startsWith(const std::string& haystack,
                          const std::string& needle);

   /**
    * Determines whether 'haystack' ends with 'needle'
    * @param haystack the string to search
    * @param needle the substring to search for
    * @return boolean indicating whether test is true
    */
   static bool endsWith(const std::string& haystack,
                        const std::string& needle);

   /**
    * Determines whether 'haystack' starts with 'needle'
    * and ends with 'needle'
    * @param haystack the string to search
    * @param needle the substring to search for
    * @return boolean indicating whether test is true
    */
   static bool startsAndEndsWith(const std::string& haystack,
                                 const std::string& needle);

   /**
    * Determines whether 'haystack' contains substring 'needle'
    * @param haystack the string to search
    * @param needle the substring to search for
    * @return boolean indicating whether test is true
    */
   static bool containsString(const std::string& haystack,
                              const std::string& needle);

   /**
    * Converts string 's' to upper case in place
    * @param s the string to convert
    */
   static void toUpperCase(std::string& s);

   /**
    * Converts string 's' to lower case in place
    * @param s the string to convert
    */
   static void toLowerCase(std::string& s);

   /**
    * Replace all occurrences of 'searchFor' with 'replaceWith' within 's'
    * @param s the string whose occurrences will be replaced
    * @param searchFor the substring to look for
    * @param replaceWith the substring to replace with
    * @return reference to updated string
    */
   static std::string& replaceAll(std::string& s,
                                  const std::string& searchFor,
                                  const std::string& replaceWith);

   /**
    * Strips leading and trailing characters 'strip' from string 's' (in place) if present
    * @param s the string to strip
    * @param strip the character to strip
    * @return reference to stripped string
    */
   static std::string& strip(std::string& s, char strip=' ');

   /**
    * Strips trailing characters 'strip' from string 's' (in place) if present
    * @param s the string to strip
    * @param strip the trailing characters to remove (if present)
    * @return reference to stripped string
    */
   static std::string& stripTrailing(std::string& s, char strip);

   /**
    * Strips leading characters 'strip' from string 's' (in place) if present
    * @param s the string to strip
    * @param strip the leading characters to remove (if present)
    * @return reference to stripped string
    */
   static std::string& stripLeading(std::string& s, char strip);

   /**
    * Trims leading spaces of string 's' (in place)
    * @param s the string to be trimmed
    * @return reference to trimmed string
    */
   static std::string& trimLeadingSpaces(std::string& s);

   /**
    * Trims leading and trailing spaces of string 's' (in place)
    * @param s the string to be trimmed
    * @return reference to trimmed string
    */
   static std::string trim(const std::string& s);

   /**
    * Pads string 's' (in place) to right with character 'padChar' to a length of 'paddedLength'
    * @param s the string to pad (if necessary)
    * @param padChar the character to use for padding
    * @param paddedLength the length of the padded string
    */
   static void padRight(std::string& s,
                        char padChar,
                        std::string::size_type paddedLength);

   /**
    * Pads string 's' (in place) to left with character 'padChar' to a length of 'paddedLength'
    * @param s the string to pad (if necessary)
    * @param padChar the character to use for padding
    * @param paddedLength the length of the padded string
    */
   static void padLeft(std::string& s,
                       char padChar,
                       std::string::size_type paddedLength);

   /**
    * Construct a string of 'length' copies of the character 'ch'
    * @param ch the character to use in constructing string
    * @param length number of characters to repeat in string
    * @return constructed string
    */
   static std::string makeStringOfChar(char ch, int length);

   /**
    * Split 's' into substrings based on delimiter 'delim'
    * @param s the string to split
    * @param delim the delimiter for splitting
    * @return vector of substrings
    */
   static std::vector<std::string> split(const std::string& s,
                                         const std::string& delim);

#if __cplusplus >= 202002L
   static int parseInt(std::string_view s);
   static long parseLong(std::string_view s);
   static float parseFloat(std::string_view s);
   static double parseDouble(std::string_view s);
   static bool startsWith(std::string_view haystack,
                          std::string_view needle);
   static bool endsWith(std::string_view haystack,
                        std::string_view needle);
   static bool containsString(std::string_view haystack,
                              std::string_view needle);
   static std::vector<std::string_view> split(std::string_view s,
                                              std::string_view delim);

#endif

};

}

#endif
