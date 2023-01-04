//   Copyright 2022-2023 symxx - caozhanhao
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
#ifndef SYMXX_ALL_TESTS_HPP
#define SYMXX_ALL_TESTS_HPP

#include "unittest.hpp"
#include "huge_test.hpp"
#include "dtoa_test.hpp"
#include "num_test.hpp"

namespace symxx::test
{
  Test get_all_tests()
  {
    Test test;
    add_huge_test(test);
    add_dtoa_test(test);
    add_num_test(test);
    return test;
  }
  
  int unittest()
  {
    try
    {
      test::Test test = test::get_all_tests();
      test.run_tests();
      test.print_results();
    }
    catch (Error &e)
    {
      std::cerr << e.get_content() << std::endl;
    }
    return 0;
  }
}
#endif