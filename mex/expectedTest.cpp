
#include <iostream>
#include <iomanip>
#include <cassert>
#include <string>
#include "Expected.h"

using namespace std;

//Demo function that checks if a string could be converted to an int via stoi
//without throwing an exception.
//Makes use of all three implicit 'basic' constructors. The use of the
//std::exception_ptr constructor is purely for testing purposes.
Expected<bool> errorIfNotInt(const std::string& s) {
  if(s.size() > 10) {
    return std::make_exception_ptr(std::out_of_range("overflow"));
  }
  for(const char c : s) {
    if(c < '0' || c > '9') {
      return std::invalid_argument("not a number");
    }
  }
  return true;
}


int main(int argc, char** argv) {

  cout << std::boolalpha;

  //Test all 'basic' constructors:
  assert(errorIfNotInt("02341").valid());
  assert(!errorIfNotInt("342341231231234").valid());
  assert(!errorIfNotInt("moo").valid());
  assert(!errorIfNotInt("moo3454").valid());

  Expected<bool> throwTest(false);
  assert(throwTest.valid());
  assert(throwTest.get() == false);
  assert(Expected<bool>(throwTest).valid()); //Copy constructor test
  assert(Expected<bool>(std::move(throwTest)).valid()); //Move constructor test
  try {
    throwTest = errorIfNotInt("343moo");
    assert(!throwTest.valid());

    assert(throwTest.hasException<std::invalid_argument>());
    assert(Expected<bool>(throwTest).hasException<std::invalid_argument>());
      //Copy constructor test

    throwTest.get();
    assert(0 && "Should have thrown!");
  } catch(...) {}

  try { throwTest.throwException(); assert(0 && "Should have thrown!"); }
  catch(...) {}

  //Swap test:
  Expected<bool> swapTest(false);
  assert(swapTest.valid());
  assert(swapTest.get() == false);
  swapTest.swap(throwTest);
  assert(!swapTest.valid());
  assert(throwTest.valid());

  //Test move construction and assignment:
  throwTest = std::move(true);
  assert(throwTest.valid());
  assert(throwTest.get() == true);

  //Test fromCode:
  auto fromCodeTest =
   Expected<int>::fromCode([&]()->Expected<int> { return stoi("23482374812"); });
  assert(!fromCodeTest.valid());
  assert(fromCodeTest.hasException<std::out_of_range>());

  fromCodeTest =
    Expected<int>::fromCode([&]()->Expected<int> { return stoi("2348812"); });
  assert(fromCodeTest.valid());
  assert(fromCodeTest.get() == 2348812);

  //Test syntactic sugar of EXPECTED_FROM_FUNCTION:
  auto sugarTest = EXPECTED_FROM_FUNCTION(stoi("23482374812"));
  assert(!sugarTest.valid());
  assert(sugarTest.hasException<std::out_of_range>());

  sugarTest = EXPECTED_FROM_FUNCTION(stoi("2348812"));
  assert(sugarTest.valid());
  assert(sugarTest.get() == 2348812);



  cout << "All tests completed successfully." << endl;

  return 0;
}
