// Copyright Paul Dardeau, SwampBits LLC 2015
// BSD License

#ifndef CHAUDIERE_AUTOPOINTER_H
#define CHAUDIERE_AUTOPOINTER_H


namespace chaudiere
{

/**
 * AutoPointer exists to do RAII (resource acquisition is initialization).
 * Old c++ had an auto_ptr that's been deprecated, and new c++ has
 * std::unique_ptr and std::shared_ptr. We've rolled our own so that we can
 * work with current (c++11) as well as old c++ compilers.
 */
template <class T>
class AutoPointer
{
public:
   AutoPointer(T objectPointer) :
      m_object(objectPointer) {
   }
   
   ~AutoPointer() {
      delete m_object;
   }
   
   T& operator->() { return m_object; }
   
   void assign(const T& copyObject) {
      if (m_object != copyObject) {
         if (m_object) {
            delete m_object;
         }
      
         m_object = copyObject;
      }
   }
   
   bool haveObject() const {
      return (nullptr != m_object);
   }
   
private:
   T m_object;
   
   // no copies
   AutoPointer(const AutoPointer<T>&);
};

}

#endif
