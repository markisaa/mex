
#pragma once

#include <string>
#include <memory>

/*
  *********************************OVERVIEW*************************************
 * This class provides several utility functions that Herb Sutter listed as being
 * 'oversights' in the C++11 standard. The source code is taken directly from
 * his slides.
 * http://channel9.msdn.com/Events/GoingNative/GoingNative-2012/C-11-VC-11-and-Beyond
 */

namespace mex {

template<class T>
auto cbegin(const T& t)->decltype(t.cbegin()) { return t.cbegin(); }
template<class T>
auto cend(const T& t)->decltype(t.cend()) { return t.cend(); }

std::string operator"" _s (const char* cstr, std::size_t sz);

template<typename T, typename ...Args>
std::unique_ptr<T> make_unique(Args&& ...args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template<typename ForwardIt, typename VAL_TYPE, typename Compare>
std::pair<ForwardIt, bool> lower_bound_find(ForwardIt first, ForwardIt last, const VAL_TYPE& value, Compare comp) {
  auto result = std::make_pair(std::lower_bound(first, last, value, comp), true);
  if(result.first == last || comp(value, *result.first))
    result.second = false;
  return result;
}

template<typename ForwardIt, typename VAL_TYPE>
std::pair<ForwardIt, bool> lower_bound_find(ForwardIt first, ForwardIt last, const VAL_TYPE& value) {
  return lower_bound_find(first, last, value, std::less<VAL_TYPE>());
}

} //namespace mex
