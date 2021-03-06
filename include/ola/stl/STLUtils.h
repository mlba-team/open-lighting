/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * STLUtils.h
 * Helper functions for dealing with the STL.
 * Copyright (C) 2012 Simon Newton
 */

#ifndef INCLUDE_OLA_STL_STLUTILS_H_
#define INCLUDE_OLA_STL_STLUTILS_H_

#include <assert.h>
#include <cstddef>
#include <map>
#include <set>
#include <vector>
#include <utility>

namespace ola {

using std::map;
using std::set;
using std::vector;

// Helper functions for sequences.
//------------------------------------------------------------------------------

/**
 * Delete the elements of a Sequence.
 */
template<typename T>
void STLDeleteElements(T *values) {
  typename T::iterator iter = values->begin();
  for (; iter != values->end(); ++iter)
    delete *iter;
  values->clear();
}

// Helper functions for associative containers.
//------------------------------------------------------------------------------

/**
 * Delete the values of an Associative Container.
 */
template<typename T>
void STLDeleteValues(T *values) {
  typename T::iterator iter = values->begin();
  for (; iter != values->end(); ++iter)
    delete iter->second;
  values->clear();
}


/**
 * Returns true if the container contains the value.
 */
template<typename T1, typename T2>
inline bool STLContains(const T1 &container, const T2 &value) {
  return container.find(value) != container.end();
}

/**
 * Extract a vector of keys from an associative container.
 */
template<typename T1>
void STLKeys(const T1 &container, vector<typename T1::key_type> *keys) {
  keys->reserve(keys->size() + container.size());
  typename T1::const_iterator iter = container.begin();
  for (; iter != container.end(); ++iter)
    keys->push_back(iter->first);
}

/**
 * Extract a vector of values from an associative container.
 */
template<typename T1>
void STLValues(const T1 &container, vector<typename T1::mapped_type> *values) {
  values->reserve(values->size() + container.size());
  typename T1::const_iterator iter = container.begin();
  for (; iter != container.end(); ++iter)
    values->push_back(iter->second);
}

/**
 * Lookup a value by key in a associative container and return a pointer to the
 * value. Returns NULL if the key doesn't exist.
 */
template<typename T1>
typename T1::mapped_type* STLFind(T1 *container,
                                  const typename T1::key_type &key) {
  typename T1::iterator iter = container->find(key);
  if (iter == container->end()) {
    return NULL;
  } else {
    return &iter->second;
  }
}

/**
 * Lookup a value by key in a associative container or pointers, or return a
 * NULL if the key doesn't exist.
 */
template<typename T1>
typename T1::mapped_type STLFindOrNull(const T1 &container,
                                       const typename T1::key_type &key) {
  typename T1::const_iterator iter = container.find(key);
  if (iter == container.end()) {
    return NULL;
  } else {
    return iter->second;
  }
}


/**
 * Sets key : value, replacing any existing value. Note if value_type is a
 * pointer, you probably don't wait this since you'll leak memory if you
 * replace a value. Returns true if the value was replaced, false if it was
 * inserted.
 */
template<typename T1>
bool STLReplace(T1 *container, const typename T1::key_type &key,
                const typename T1::mapped_type &value) {
  std::pair<typename T1::iterator, bool> p = container->insert(
      typename T1::value_type(key, value));
  if (!p.second) {
    p.first->second = value;
    return true;
  }
  return false;
}


/**
 * Insert an key : value into a map where value is a pointer. This replaces the
 * previous value if it exists and deletes it.
 */
template<typename T1>
void STLReplaceAndDelete(T1 *container, const typename T1::key_type &key,
                         const typename T1::mapped_type &value) {
  std::pair<typename T1::iterator, bool> p = container->insert(
      typename T1::value_type(key, value));
  if (!p.second) {
    delete p.first->second;
    p.first->second = value;
  }
}


/**
 * Insert a value into a container only if this value doesn't already exist.
 * Returns true if the key was inserted, false if the key already exists.
 */
template<typename T1>
bool STLInsertIfNotPresent(T1 *container,
                           const typename T1::value_type &value) {
  return container->insert(value).second;
}


/**
 * Insert an key : value into a map only if a value for this key doesn't
 * already exist.
 * Returns true if the key was inserted, false if the key already exists.
 */
template<typename T1>
bool STLInsertIfNotPresent(T1 *container, const typename T1::key_type &key,
                           const typename T1::mapped_type &value) {
  return container->insert(typename T1::value_type(key, value)).second;
}


/**
 * Insert an key : value into a map or die if the key already exists.
 */
template<typename T1>
void STLInsertOrDie(T1 *container, const typename T1::key_type &key,
                    const typename T1::mapped_type &value) {
  assert(container->insert(typename T1::value_type(key, value)).second);
}


/**
 * Remove a item from a container.
 * @returns true if the item was removed, false otherwise.
 */
template<typename T1>
bool STLRemove(T1 *container, const typename T1::key_type &key) {
  return container->erase(key);
}

/**
 * Lookup a value by key in a associative container. If the value exists, it's
 * removed from the container, the value placed in value and true is returned.
 * If the value doesn't exist this returns false.
 */
template<typename T1>
bool STLLookupAndRemove(T1 *container,
                        const typename T1::key_type &key,
                        typename T1::mapped_type *value) {
  typename T1::iterator iter = container->find(key);
  if (iter == container->end()) {
    return false;
  } else {
    *value = iter->second;
    container->erase(iter);
    return true;
  }
}


/**
 * Lookup a value by key in a associative container. If the value exists, it's
 * deleted, removed from the container and true is returned.
 * If the value doesn't exist this returns false.
 */
template<typename T1>
bool STLRemoveAndDelete(T1 *container, const typename T1::key_type &key) {
  typename T1::iterator iter = container->find(key);
  if (iter == container->end()) {
    return false;
  } else {
    delete iter->second;
    container->erase(iter);
    return true;
  }
}


/**
 * Similar to STLLookupAndRemove but this operates on containers of pointers.
 * If the key exists, we remove it and return the value, if the key doesn't
 * exist we return NULL.
 */
template<typename T1>
typename T1::mapped_type STLLookupAndRemovePtr(
    T1 *container,
    const typename T1::key_type &key) {
  typename T1::iterator iter = container->find(key);
  if (iter == container->end()) {
    return NULL;
  } else {
    typename T1::mapped_type value = iter->second;
    container->erase(iter);
    return value;
  }
}
}  // namespace ola
#endif  // INCLUDE_OLA_STL_STLUTILS_H_
