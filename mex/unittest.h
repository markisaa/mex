
#pragma once

#include <vector>
#include <functional>
#include <stdexcept>

/*
 * This header is meant to facilitate easy creation and usage of unit tests in
 * your code. This is inspired by the D language's unittest blocks.
 *
 * Defining a unit test:
 *
 * A unit test may be declared inside any single scope block. It will be added
 * to the master test list by the time that scope has been terminated at runtime.
 * All tests declared at namespace level will be added prior to reaching 'main'
 * with no guaranteed ordering.
 *
 * In any such scope, simply write:
MEX_UNIT_TEST
  <arbitrary code here>
MEX_END_UNIT_TEST

*
* to define a unit test. Unit tests are semantically no different than normal
* functions.
*
* Running unit tests:
*
* The unit testing suite can be ran via the runUnitTests function if and only if
* the program has been compiled with the UNIT_TEST symbol defined.
* To define the UNIT_TEST symbol, add -DUNIT_TEST to your g++ invocation(s):
g++ -DUNIT_TEST -std=c++11 foo.cpp

* This macro makes it convenient to always call unittest::runUnitTests() as the first
* line of main, knowing it will only be run when intended.
*
*
* Example program:

int main() {
  unittest::runUnitTests();
}

int square(int x) {
  return x*x;
}

MEX_UNIT_TEST
  assert(square(1) == 1);
  assert(square(2) == 4);
  assert(square(3) == 9);
  assert(square(4) == 16);
MEX_END_UNIT_TEST

*/



#ifdef UNIT_TEST
#define MEX_RUN_UNIT_TESTS
#endif

#define MEX_UT_DETAIL_CONCAT_(a, b) a##b
#define MEX_UT_DETAIL_CONCAT(a, b) MEX_UT_DETAIL_CONCAT_(a, b)

#define MEX_UNIT_TEST \
  namespace mex_ut { __attribute__((unused)) static bool MEX_UT_DETAIL_CONCAT(UT,__LINE__) = unittest::detail::registerUnitTest([](){

//Could change to a single macro with TEST(code), but emacs likes this less...
#define MEX_END_UNIT_TEST }); }

namespace unittest {

  void runUnitTests();

  struct ExpectationFailed : std::runtime_error {
    ExpectationFailed();
  };
  void expect_true(bool condition);

  template<typename FUNC>
  void expect_exception(FUNC func) {
    try {
      func();
    } catch(const ExpectationFailed&) {
      throw;
    } catch(...) {
      return;
    }
    throw ExpectationFailed{};
  }

  template<typename EX, typename FUNC>
  void expect_exception(FUNC func) {
    try {
      func();
    } catch(const EX&) {
      return;
    }
    throw ExpectationFailed{};
  }

  namespace detail {
    using UnitTestFunction = std::function<void()>;
    bool registerUnitTest(UnitTestFunction func);
  } //namespace detail


} //namespace unittest
