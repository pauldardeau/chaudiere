// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_STRINGTOKENIZER_H
#define CHAUDIERE_STRINGTOKENIZER_H

#include <string>
#include <vector>

namespace chaudiere
{

/**
 * StringTokenizer is very similar to Java's StringTokenizer.
 * @see StrUtils::split for similar functionality.
 */
class StringTokenizer
{
   public:
      /**
       * Constructs a StringTokenizer and tokenizes the specified string
       * @param s the string to tokenize
       */
      explicit StringTokenizer(const std::string& s);

      /**
       * Constructs a StringTokenizer with the string to tokenize and the delimiter
       * @param s the string to tokenize
       * @param delimiter the delimiter of the tokens
       */
      StringTokenizer(const std::string& s,
                      const std::string& delimiter);

      /**
       * Destructor
       */
      ~StringTokenizer();

      /**
       */
      void parse();

      /**
       * Determines whether more tokens are present
       * @return boolean indicating if there are more tokens available
       */
      bool hasMoreTokens() const;

      /**
       * Retrieves the next available token
       * @throw std::out_of_range
       * @return the next token
       */
      const std::string& nextToken();

      /**
       * Retrieves the number of tokens found
       * @return the number of tokens
       */
      std::size_t countTokens() const;

      /**
       * Retrieve list of tokens found
       * @return the list of parsed tokens
       */
      const std::vector<std::string>& getTokens() const;



   private:
      const std::string& m_withTokens;
      const std::string m_delimiter;
      std::vector<std::string> m_tokens;
      std::size_t m_numberTokens;
      std::size_t m_indexToken;

      //disallow copies
      StringTokenizer(const StringTokenizer&);
      StringTokenizer& operator=(const StringTokenizer&);

};

}

#endif


