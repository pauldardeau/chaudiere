// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_OSUTILS_H
#define CHAUDIERE_OSUTILS_H

#include <string>

namespace chaudiere
{
   
   /**
    * OSUtils is a collection of OS related utilities that are frequently used
    */
   class OSUtils
   {
   public:
      /**
       * Retrieves the current directory
       * @return the current directory
       */
      static std::string getCurrentDirectory();
      
      /**
       * Joins (concatenates) a directory path and file name and will include the path delimiter if necessary
       * @param directory the path for the directory
       * @param fileName the name of the file
       * @return joined directory path and file name
       */
      static std::string pathJoin(const std::string& directory, const std::string& fileName);
      
      /**
       * Determines if the specified file path exists
       * @param filePath the file path whose existence is to tested
       * @return boolean indicating if the file path exists
       */
      static bool pathExists(const std::string& filePath);

   };

}

#endif
