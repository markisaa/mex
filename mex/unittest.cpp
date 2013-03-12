
#include "unittest.h"

using std::vector;

namespace unittest {

vector<mex_unitTestFunction> mex_unitTests{};

bool registerUnitTest(mex_unitTestFunction func) {
  mex_unitTests.push_back(func);
  return true;
}


void runUnitTests() {
#ifdef MEX_RUN_UNIT_TESTS
  for(auto& test : mex_unitTests)
    test();
#endif
}

} //namespace unittest
