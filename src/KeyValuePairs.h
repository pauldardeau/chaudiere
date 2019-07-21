// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_KEYVALUEPAIRS_H
#define CHAUDIERE_KEYVALUEPAIRS_H

#include <string>
#include <vector>
#include <unordered_map>

namespace chaudiere
{

/**
 * KeyValuePairs is a collection of key/value pairs where the key and the
 * value are both strings.
 */
class KeyValuePairs
{
   public:
      /**
       * Default constructor
       */
      KeyValuePairs();
   
      /**
       * Copy constructor
       * @param copy the source of the object copy
       */
      KeyValuePairs(const KeyValuePairs& copy);
   
      /**
       * Destructor
       */
      ~KeyValuePairs();

      /**
       * Assignment operator
       * @param copy source of object copy
       * @return updated object after the copy.
       */
      KeyValuePairs& operator=(const KeyValuePairs& copy);
   
      /**
       * Retrieves the keys contained within the collection
       * @param keys the vector to be populated with collection keys
       */
      void getKeys(std::vector<std::string>& keys) const;

      /**
       * Determines if the specified key exists within the collection.
       * @param key the key whose existing is being tested
       * @return boolean indicating whether the specified key exists
       */
      bool hasKey(const std::string& key) const;

      /**
       * Retrieves the value associated with the specified key
       * @param key the key whose value is requested
       * @throw InvalidKeyException
       * @return the value associated with the specified key
       */
      const std::string& getValue(const std::string& key) const;

      /**
       * Adds a key/value pair to the collection
       * @param key the key of the pair being added
       * @param value the value of the pair being added
       */
      void addPair(const std::string& key,
                   const std::string& value);

      /**
       * Removes a pair identified by the specified key
       * @param key the key of the pair to be removed
       * @return boolean indicating whether a pair was removed
       */
      bool removePair(const std::string& key);

      /**
       * Removes all key/value pairs within the collection
       */
      void clear();
   
      /**
       * Retrieves the number of pairs contained within the collection
       * @return the number of key/value pairs contained
       */
      std::size_t size() const;
   
      /**
       * Determines if the collection is empty
       * @return boolean indicating whether the collection is empty
       */
      bool empty() const;
   

   private:
      std::unordered_map<std::string, std::string> m_keyValues;
};

}

#endif

