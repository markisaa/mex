
#include <vector>
#include <functional>

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

 * Or the equivalent shortform:
MEX_UT
  <arbitrary code here>
MEX_EUT

 *
 * to define a unit test. Unit tests are semantically no different than normal
 * functions.
 *
 * Running unit tests:
 *
 * The unit testing suite can be ran via the runUnitTests function if the program
 * has been compiled with the UNIT_TEST symbol defined by the compiler. This makes
 * it convenient to simply call the function as the first line of main, knowing it
 * will only be run when intended.
 *
 *
 * Example program:

int square(int x) {
  return x*x;
}

MEX_UNIT_TEST
  assert(square(1) == 1);
  assert(square(2) == 4);
  assert(square(3) == 9);
  assert(square(4) == 16);
MEX_END_UNIT_TEST

int main() {
  unittest::runUnitTests();
}
 *
 */



#ifdef UNIT_TEST
#define MEX_RUN_UNIT_TESTS
#endif

#define MEX_UT_CONCAT_(a, b) a##b
#define MEX_UT_CONCAT(a, b) MEX_UT_CONCAT_(a, b)
#define MEX_UNIT_TEST \
  static bool MEX_UT_CONCAT(UT,__LINE__) = unittest::registerUnitTest([](){
#define MEX_END_UNIT_TEST });

#define MEX_UT MEX_UNIT_TEST
#define MEX_EUT MEX_END_UNIT_TEST

namespace unittest {

using mex_unitTestFunction = std::function<void()>;
extern std::vector<mex_unitTestFunction> mex_unitTests;


bool registerUnitTest(mex_unitTestFunction func);


void runUnitTests();

} //namespace unittest
