// Copyright Paul Dardeau, SwampBits LLC 2015
// BSD License

#ifndef CHAUDIERE_BYTEBUFFER_H
#define CHAUDIERE_BYTEBUFFER_H

#include <stdlib.h>
#include <string.h>


namespace chaudiere
{

/**
 * ByteBuffer is a low-level class that manages a simple memory data buffer.
 */
class ByteBuffer
{
public:
   /**
    * Constructs a new ByteBuffer
    */
   ByteBuffer() :
      m_buffer(nullptr),
      m_bufferSize(0) {
   }
   
   /**
    * Constructs a new ByteBuffer of the specified size
    * @param bufferSize the size needed for the buffer
    */
   explicit ByteBuffer(const std::size_t bufferSize) noexcept :
      m_bufferSize(bufferSize) {
      if (m_bufferSize > 0) {
         m_buffer = ::calloc(1, m_bufferSize);
      } else {
         m_buffer = nullptr;
      }
   }
   
   /**
    * Copy constructor
    * @param copy the source of the copy
    */
   ByteBuffer(const ByteBuffer& copy) noexcept :
      m_bufferSize(copy.m_bufferSize) {
      if (m_bufferSize > 0) {
         m_buffer = ::calloc(1, m_bufferSize);
         ::memcpy(m_buffer, copy.m_buffer, m_bufferSize);   
      } else {
         m_buffer = nullptr;
      }
   }
   
   /**
    * Destructor
    */
   ~ByteBuffer() noexcept {
      clear();
   }
   
   /**
    * Copy operator
    * @param copy the source of the copy
    * @return reference to the target of the copy
    */
   ByteBuffer& operator=(const ByteBuffer& copy) noexcept {
      if (this == &copy) {
         return *this;
      }
      
      clear();
      m_bufferSize = copy.m_bufferSize;
      if (m_bufferSize > 0) {
         m_buffer = ::calloc(1, m_bufferSize);
         ::memcpy(m_buffer, copy.m_buffer, m_bufferSize);   
      }
      
      return *this;
   }
   
   /**
    * Take on ownership of an existing data buffer
    * @param sourceBuffer the new data buffer
    */
   void take(void* sourceBuffer, std::size_t bufferSize) {
      if (m_buffer != nullptr) {
         ::free(m_buffer);
         m_bufferSize = 0;
      }
      
      m_buffer = sourceBuffer;
      m_bufferSize = bufferSize;
   }
   
   /**
    * Releases ownership of existing buffer
    * @return pointer to released data buffer
    */
   void* release() {
      void* releasedBuffer = m_buffer;
      m_buffer = nullptr;
      m_bufferSize = 0;
      return releasedBuffer;
   }
   
   /**
    * Retrieve pointer to raw data buffer
    * @return pointer to raw data buffer
    */
   void* data() noexcept {
      return m_buffer;
   }
   
   /**
    * Retrieve size of data buffer
    * @return size of data buffer in bytes
    */
   std::size_t size() const noexcept {
      return m_bufferSize;
   }
   
   /**
    * Frees existing buffer
    */
   void clear() noexcept {
      if (m_buffer != nullptr) {
         ::free(m_buffer);
         m_buffer = nullptr;
         m_bufferSize = 0;
      }
   }


private:
   void* m_buffer;
   std::size_t m_bufferSize;
};

}

#endif
