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
       * Determines if the specified directory exists
       * @param directory the directory whose existence is to be tested
       * @return boolean indicating if the directory exists
       */
      static bool directoryExists(const std::string& directory);
      
      /**
       * Joins (concatenates) a directory path and file name and will include the path delimiter if necessary
       * @param directory the path for the directory
       * @param fileName the name of the file
       * @return joined directory path and file name
       */
      static std::string pathJoin(const std::string& directory,
                                  const std::string& fileName);
      
      /**
       * Determines if the specified file path exists
       * @param filePath the file path whose existence is to be tested
       * @return boolean indicating if the file path exists
       */
      static bool pathExists(const std::string& filePath);
      
      /**
       * Creates a new directory of the specified name
       * @param directory name of new directory to create
       * @return boolean indicating if the directory was created
       */
      static bool createDirectory(const std::string& directory);

     /**
      * Creates a new private directory of the specified name
      * @param directory name of new directory to create
      * @return boolean indicating if the directory was created
      */
     static bool createPrivateDirectory(const std::string& directory);
      
      /**
       *
       * @param inCrc32
       * @param buf
       * @param bufLen
       * @return
       */
      static unsigned long crc32ForBuffer(unsigned long inCrc32,
                                          const void *buf,
                                          size_t bufLen);

      /**
       * Computes the CRC-32 value for the specified file
       * @param filePath the path to the file
       * @param crc32 will be populated with the CRC-32 value as a hex string
       * @return boolean indicating if the CRC value was obtained for the file
       */
      static bool crc32ForFile(const std::string& filePath,
                               std::string& crc32);

   };

}

#endif
