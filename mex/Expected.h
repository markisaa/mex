
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <utility>
#include <type_traits>

/*
 *********************************OVERVIEW*************************************
 * This class is inspired and mostly copied from Andrei Alexancrescu's
 * presentation on "Systematic Error Handling in C++" at C++ and Beyond 2012.
 * Viewable here: http://channel9.msdn.com/Shows/Going+Deep/C-and-Beyond-2012-Andrei-Alexandrescu-Systematic-Error-Handling-in-C
 *
 * The class has been modified to achieve more natural use semantics at the
 * cost of requiring that the expected TYPE is not an std::exception and
 * minimalizing support for exceptions that do not inherit std::exception, as
 * vaguely suggested by Herb Sutter. These modifications along with all
 * documentation not including the overview and first code example are
 * authored by Mark Isaacson.
 * I make no claim to ownership of the 'original content', whose rights are
 * under the control of Andrei Alexandrescu. Further - all of Mark's changes
 * and documentation are presented "as-is" without any guarantee of
 * correctness or support. You may consider these alterations as being
 * available under the same legal restrictions and terms as the
 * 'original content'.
 *
 *
 * This class is meant to enable a cleaner, more versatile, mode of error
 * handling by offering the following features:
 *    * Associates errors with computational goals
 *    * Naturally allows multiple exceptions in flight
 *    * Switch between "error handling" and "exception throwing" styles
 *    * Teleportation possible
 *        * Across thread boundaries
 *        * Across nothrow subsystem boundaries
 *        * Across time: save now, throw later
 *    * Collect, group, combine exceptions
 *
 * The key idea that allows this is that Expected<TYPE> is either a TYPE or the
 * exception preventing its creation.
 *
 *
 ********************************EXAMPLE 1*************************************
 * We first explore the semantics of Expected<TYPE> in a function 'parseInt',
 * which attempts to convert a string to an int. The function will return
 * an int if successful and an exception if not (contained within an
 * Expected<int> in either case).
 * We achieve "normal" return syntax in addition to providing an
 * error-code-like interface.
 *
 * Consider:
 *
  Expected<int> parseInt(const std::string& s) {
    int result;
    ...
    if (nonDigit) {
      return std::invalid_argument("not a number");
    }
    ...
    if (tooManyDigits) {
    return std::out_of_range("overflow");
    }
    ...
    return result;
  }
 *
 * We can then call parseInt and check to see whether or not the result is
 * 'valid' (that no exception occurred):
 *
   parseInt("12312").valid();           //True
   parseInt("23482374812").valid();     //False
   parseInt("moo").valid();             //False
 *
 * Further - we can retrieve the value if it is valid:
 *
   int value = parseInt("12312").get(); //value holds int(12312)
 *
 * And we can determine the nature of the exception either by calling
 * the get member function on an invalid Expected<TYPE> and catching the result
 * or by calling hasException:
 *
   parseInt("23482374812").hasException<std::out_of_range>();     //True
   parseInt("23482374812").hasException<std::invalid_argument>(); //False
 *
 *
 ********************************EXAMPLE 2*************************************
 * We will now look at some syntactic sugar that makes integrating this form of
 * error handling with existing libraries simple.
 *
 * Suppose that instead of writing our own parseInt function, we instead wanted
 * to take advantage of std::stoi, but we don't want to explicitly write a try
 * catch block nor do we want any thrown exceptions to propagate past us in the
 * stack - we can capture these semantics with Expected<TYPE>::fromCode:
 *
   auto ret =
     Expected<int>::fromCode([&]()->Expected<int> { return stoi("23482374812"); });
 *
 * Calling stoi wrapped in this fashion will yield the same semantic results
 * as our hypothetical parseInt function in the first example. The only
 * difference here is that the exception *will* be thrown, then caught, and
 * then put into an Expected<TYPE> - which means there will be a performance
 * hit compared to a function designed to work natively with Expected<TYPE>.
 *
 * Note that fromCode as used above only takes input that is callable without
 * arguments. Unfortunately I can't think of a way to get semantics on the
 * order of:
 * auto ret = Expected<int>::fromCode(stoi("23482374812")); //NOT VALID!
 * without resorting to pre-processor shenangians - so I have gone down that
 * route to provide something similar:
 *
   auto ret = EXPECTED_FROM_FUNCTION(stoi("23482374812"));
 *
 * Will create ret with type Expected<int>, or more generally, Expected<TYPE>
 * where TYPE is the retrun type of the provided FUNCTION.
 *
 *
 *********************************ODDITIES*************************************
 * A brief summary of what might be unexpected behavior:
 * 1) Expected<TYPE>'s TYPE cannot be anything that inherits std::exception.
 * 2) Expected<TYPE> provides minimal support (almost none) for exception types
 *    that do not inherit std::exception. (fromException is the only means
 *    around this).
 * Item's 1 and 2 enable cleaner semantics when instantiating an Expected<TYPE>.
 * Further - it is generally considered poor style to throw primitives and this
 * class is easily modified to serve a custom exception hierarchy.
 * 3) Calling the get member function on an Expected<TYPE> that is not valid
 *    will result in an exception being thrown.
 * 4) The hasException method is... exceptionally... slow. The only way to
 *    determine the nature of the exception is to throw it, and so repeated
 *    calls to hasException is a poor idea - consider calling throwException
 *    instead and catching it manually.
 */

//Enabled only for non-exception TYPEs (don't facilitate Expecting an exception).
template<typename TYPE,
  typename ENABLE =
  typename std::enable_if<!std::is_base_of<std::exception, TYPE>::value
                       && !std::is_base_of<std::exception_ptr, TYPE>::value
>::type>
class Expected {
public:
  Expected(const TYPE& rhs); //Construct from TYPE.

  template<typename EX,
  typename CHECK = typename std::enable_if<std::is_base_of<std::exception, EX>::value>::type>
  Expected(const EX& ex); //Construct from class derived from std::exception.

  Expected(std::exception_ptr exptr); //Construct from std::exception_ptr

  Expected(TYPE&& rhs); //Move construct from TYPE.

  Expected(const Expected& rhs); //Copy constructor.

  Expected(Expected&& rhs); //Move constructor.

  ~Expected();

  Expected<TYPE>& operator=(const Expected<TYPE> &rhs);

  Expected<TYPE>& operator=(Expected<TYPE> &&rhs);

  void swap(Expected& rhs);

  bool valid() const; //Returns true if holds TYPE, false if holds an exception

  TYPE& get(); //Returns value of held TYPE if valid, else throws exception.
  const TYPE& get() const;

  void throwException() const; //Throws the held exception if there is one.

  template<typename EX>
  bool hasException() const; //Allows you to query for the exception type.

  static Expected<TYPE> fromException();
    //If used within a catch statement, this will construct an Expected<TYPE>
    //that holds whatever exception is 'currently in flight' (whatever you just
    //caught).

  template<typename FUNC>
  static Expected fromCode(FUNC func);
    //Syntactic sugar allowing you to wrap functions that use normal exception
    //handling code.



private:
  union {
    TYPE ham;
    std::exception_ptr spam;
  };
  bool gotHam;
};


/******************************************************************************
 ******************************************************************************
 *******************************INLINE FUNCTIONS*******************************
 ******************************************************************************
 *****************************************************************************/

template<typename TYPE, typename ENABLE>
Expected<TYPE, ENABLE>::Expected(const TYPE& rhs) : ham(rhs), gotHam(true) {}

template<typename TYPE, typename ENABLE>
template<typename EX, typename CHECK>
Expected<TYPE, ENABLE>::Expected(const EX& ex)
  : spam(std::make_exception_ptr(ex)), gotHam(false)
{
  if(typeid(ex) != typeid(EX)) {
    throw std::invalid_argument("slicing detected");
  }
}

template<typename TYPE, typename ENABLE>
Expected<TYPE, ENABLE>::Expected(std::exception_ptr exptr)
  : spam(std::move(exptr)), gotHam(false) {}

template<typename TYPE, typename ENABLE>
Expected<TYPE, ENABLE>::Expected(TYPE&& rhs)
  : ham(std::move(rhs)), gotHam(true) {}

template<typename TYPE, typename ENABLE>
Expected<TYPE, ENABLE>::Expected(const Expected& rhs) : gotHam(rhs.gotHam) {
  if(gotHam) new(&ham) TYPE(rhs.ham);
  else new(&spam) std::exception_ptr(rhs.spam);
}

template<typename TYPE, typename ENABLE>
Expected<TYPE, ENABLE>::Expected(Expected&& rhs) : gotHam(rhs.gotHam) {
  if(gotHam) new(&ham) TYPE(std::move(rhs.ham));
  else new(&spam) std::exception_ptr(std::move(rhs.spam));
}

template<typename TYPE, typename ENABLE>
Expected<TYPE, ENABLE>::~Expected() {
  using std::exception_ptr;
    //Necessary because scope operator not allowed after . operator.
  if(gotHam) ham.~TYPE();
  else spam.~exception_ptr();
}

template<typename TYPE, typename ENABLE>
Expected<TYPE>& Expected<TYPE, ENABLE>::operator=(const Expected<TYPE> &rhs) {
  gotHam = rhs.gotHam;
  if(gotHam) new(&ham) TYPE(rhs.ham);
  else new(&spam) std::exception_ptr(rhs.spam);
  return *this;
}

template<typename TYPE, typename ENABLE>
Expected<TYPE>& Expected<TYPE, ENABLE>::operator=(Expected<TYPE> &&rhs) {
  if(rhs.gotHam) ham = std::move(rhs.ham);
  else spam = std::move(rhs.spam);
  gotHam = rhs.gotHam;
  return *this;
}

template<typename TYPE, typename ENABLE>
void Expected<TYPE, ENABLE>::swap(Expected& rhs) {
  if(gotHam) {
    if(rhs.gotHam) {
      //Put std::swap in the namespace lookup, but allow specializations.
      using std::swap;
      swap(ham, rhs.ham);
    } else {
      auto t = std::move(rhs.spam);
      new(&rhs.ham) TYPE(std::move(ham));
      new(&spam) std::exception_ptr(t);
      std::swap(gotHam, rhs.gotHam);
    }
  } else {
    if(rhs.gotHam) {
      rhs.swap(*this); //Single recursive call to symmetric case to be lazy.
    } else {
      spam.swap(rhs.spam);
      std::swap(gotHam, rhs.gotHam);
    }
  }
}

template<typename TYPE, typename ENABLE>
Expected<TYPE> Expected<TYPE, ENABLE>::fromException() {
  return Expected<TYPE>(std::current_exception());
}

template<typename TYPE, typename ENABLE>
bool Expected<TYPE, ENABLE>::valid() const {
  return gotHam;
}

template<typename TYPE, typename ENABLE>
TYPE& Expected<TYPE, ENABLE>::get() {
  throwException();
  return ham;
}

template<typename TYPE, typename ENABLE>
const TYPE& Expected<TYPE, ENABLE>::get() const {
  throwException();
  return ham;
}

template<typename TYPE, typename ENABLE>
void Expected<TYPE, ENABLE>::throwException() const {
  if(!gotHam) std::rethrow_exception(spam);
}

template<typename TYPE, typename ENABLE>
template<typename EX>
bool Expected<TYPE, ENABLE>::hasException() const {
  try {
    throwException();
  } catch(const EX& object) {
    return true;
  } catch(...) {
  }
  return false;
}

template<typename TYPE, typename ENABLE>
template<typename FUNC>
Expected<TYPE, ENABLE> Expected<TYPE, ENABLE>::fromCode(FUNC func) {
  try {
    return Expected(func());
  } catch(...) {
    return fromException();
  }
}

#define EXPECTED_FROM_FUNCTION(FUNCTION)                                     \
  Expected<decltype(FUNCTION)>::fromCode([&]()->Expected<decltype(FUNCTION)> \
                                          { return FUNCTION; }               \
                                        )
