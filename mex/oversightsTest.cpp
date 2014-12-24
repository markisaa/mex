
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>
#include <functional>
#include <type_traits>
#include <cassert>

#include "std_oversights.h"
#include "unittest.h"

using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::equal;
using std::islower;
using std::function;
using std::greater_equal;
using std::less;
using std::is_same;
using std::result_of;
using std::begin;
using std::end;

using mex::cbegin;
using mex::cend;
using mex::size;
using mex::empty;
using mex::data;
using mex::make_unique;
using mex::not_fn;
using mex::operator"" _s;

int main(int argc, char** argv) {
  unittest::runUnitTests();
  cout << "All tests completed successfully." << endl;
  return 0;
}

MEX_UNIT_TEST
  vector<int> v { 1, 2, 3, 4, 5 };
  assert(cbegin(v) == v.cbegin());
  assert(cend(v) == v.cend());
  assert(size(v) == v.size());
  assert(empty(v) == v.empty());
  assert(data(v) == v.data());
MEX_END_UNIT_TEST

MEX_UNIT_TEST
  int arr[5] = { 1, 2, 3, 4, 5 };
  assert(cbegin(arr) == arr);
  assert(cend(arr) == arr + 5);
  assert(size(arr) == 5);
  assert(empty(arr) == false);
  assert(data(arr) == arr);
MEX_END_UNIT_TEST

MEX_UNIT_TEST
  auto initList = { 1, 2, 3, 4, 5 };
  assert(size(initList) == initList.size());
  assert(empty(initList) == initList.size() == 0);
  assert(data(initList) == initList.begin());
MEX_END_UNIT_TEST

MEX_UNIT_TEST
  assert("Test"_s == "Test");
  assert("Test"_s == "Test"_s);
MEX_END_UNIT_TEST

MEX_UNIT_TEST
  auto ptr = make_unique<int>(5);
  assert(ptr);
  assert(*ptr == 5);
  ptr.reset();
  assert(!ptr);
MEX_END_UNIT_TEST

//Necessary to overcome overload resolution silliness.
int unambiguousIsLower(char c) {
  return islower(c);
}

MEX_UNIT_TEST
  auto wrapper = [](char c) { return unambiguousIsLower(c); };
  auto negatedWithLambda = [](char c) { return !unambiguousIsLower(c); };
  auto negatedWithNotFn = not_fn(unambiguousIsLower);
  auto negatedWithNotFnOnStdFunction =
    not_fn(function<int(char)>{unambiguousIsLower});

  auto testString = "aBcDEFghIjKLmNop"_s;
  for(auto c : testString) {
    assert(islower(c) == wrapper(c));
    assert(!islower(c) == negatedWithLambda(c));
    assert(!islower(c) == negatedWithNotFn(c));
    assert(!islower(c) == negatedWithNotFnOnStdFunction(c));
  }
MEX_END_UNIT_TEST

MEX_UNIT_TEST
  auto v = vector<int>{ 5, 2, 3, 1, 6, 7, 9, 10, 1, 1, 3, 8 };
  auto vCopy = v;

  sort(begin(v), end(v), greater_equal<int>{});
  sort(begin(vCopy), end(vCopy), not_fn(less<int>{}));
  assert(equal(begin(v), end(v), begin(vCopy)));
MEX_END_UNIT_TEST

MEX_UNIT_TEST
  struct NonBoolNegation {
    string operator!() { return "foobar"_s; }

    static NonBoolNegation factory() { return NonBoolNegation{}; }
  };

  using NotFnImplType = decltype(not_fn(NonBoolNegation::factory));
  using negationResultType = typename result_of<NotFnImplType()>::type;
  static_assert(
    is_same<string, negationResultType>::value,
    "not_fn broken on types that return non-bool.");
MEX_END_UNIT_TEST
