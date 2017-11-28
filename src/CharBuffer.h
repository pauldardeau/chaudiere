// Copyright Paul Dardeau, SwampBits LLC 2015
// BSD License

#ifndef CHAUDIERE_CHARBUFFER_H
#define CHAUDIERE_CHARBUFFER_H

#include <stdlib.h>
#include <string.h>


namespace chaudiere
{

/**
 * CharBuffer is a low-level class that manages a simple memory data buffer.
 */
class CharBuffer
{
public:
   /**
    */
   CharBuffer() :
      m_buffer(NULL),
      m_bufferSize(0) {
   }

   /**
    * Constructs a new CharBuffer of the specified size
    * @param bufferSize the size needed for the buffer
    */
   explicit CharBuffer(const std::size_t bufferSize) :
      m_buffer(NULL),
      m_bufferSize(bufferSize) {
      if (m_bufferSize > 0) {
         allocateBuffer(bufferSize);
      }
   }
   
   /**
    * Destructor
    */
   ~CharBuffer() {
      if (m_buffer != NULL) {
         delete [] m_buffer;
      }
   }

   void nullAt(const std::size_t offset) {
      if (offset < m_bufferSize) {
         m_buffer[offset] = '\0';
      }
   }

   void allocateBuffer(const std::size_t bufferSize) {
      if (bufferSize > 0) {
         m_buffer = new char[bufferSize];
         memset(m_buffer, 0, bufferSize);
         m_bufferSize = bufferSize;
      }
   }

   void ensureCapacity(const std::size_t bufferSize) {
      if (NULL == m_buffer) {
         allocateBuffer(bufferSize);
      } else {
         if (bufferSize > m_bufferSize) {
            char* largerBuffer = new char[bufferSize];
            memcpy(largerBuffer, m_buffer, m_bufferSize);
            delete [] m_buffer;
            m_buffer = largerBuffer;
            m_bufferSize = bufferSize;
         }
      }
   }
   
   /**
    * Retrieve pointer to raw data buffer
    * @return pointer to raw data buffer
    */
   char* data() {
      return m_buffer;
   }
   
   /**
    * Retrieve size of data buffer
    * @return size of data buffer in bytes
    */
   std::size_t size() const {
      return m_bufferSize;
   }
   

private:
   char* m_buffer;
   std::size_t m_bufferSize;

   // disallow copies
   CharBuffer(const CharBuffer& copy);
   CharBuffer& operator=(const CharBuffer& copy);
};

}

#endif
