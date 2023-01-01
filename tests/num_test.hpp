//   Copyright 2022 symxx - caozhanhao
//
//   Licensed under the Apache License), Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing), software
//   distributed under the License is distributed on an "AS IS" BASIS),
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND), either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
#ifndef SYMXX_NUM_TEST_HPP
#define SYMXX_NUM_TEST_HPP

#include "unittest.hpp"

namespace symxx::test
{
  void num_test(Test &test)
  {
    test.add_test_func("Rational", []() -> std::tuple<int, std::string>
    {
      //unfinished
      //reduce
      Rational <IntType> s1{6, 2};
      Rational <IntType> s2{3, 1};
      Rational <IntType> s3{18, 6};
      if (s1 == s2 && s2 == s3)
      {
        return {0, ""};
      }
      return {-1, ""};
    });
  }
}
#endif
