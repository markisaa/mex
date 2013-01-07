
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


template<typename T, typename ...Args>
std::unique_ptr<T> make_unique(Args&& ...args)
{
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

} //namespace mex
