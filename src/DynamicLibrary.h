// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef DYNAMICLIBRARY_H
#define DYNAMICLIBRARY_H

#include <string>

/**
 * DynamicLibrary is a wrapper around dlopen, dlsym, dlclose.
 */
class DynamicLibrary
{
   public:
      /**
       * Default constructor
       */
      DynamicLibrary() noexcept;
   
      /**
       * Opens the specified dynamic library
       * @param libraryName the name of the dynamic library file to open
       * @throw BasicException
       */
      explicit DynamicLibrary(const std::string& libraryName);
   
      /**
       * Destructor (closes the library if open)
       */
      ~DynamicLibrary() noexcept;

      /**
       * Retrieves a function pointer for the specified function name
       * @param functionName the name of the function in the dynamic library
       * @return pointer to function (if present), or nullptr if not
       */
      void* resolve(const std::string& functionName) noexcept;
   
      /**
       * Opens the specified dynamic library file
       * @param libraryName the name of the dynamic library file to open
       * @return boolean indicating whether the dynamic library was opened
       */
      bool open(const std::string& libraryName) noexcept;
   
      /**
       * Closes the dynamic library (if open)
       */
      void close() noexcept;
   
      // copies not allowed
      DynamicLibrary(const DynamicLibrary&) = delete;
      DynamicLibrary(DynamicLibrary&&) = delete;
      DynamicLibrary& operator=(const DynamicLibrary&) = delete;
      DynamicLibrary& operator=(DynamicLibrary&&) = delete;

   private:
      typedef void* HDLL;
      HDLL m_hDll;
};

#endif

