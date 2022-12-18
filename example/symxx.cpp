//   Copyright 2021-2022 symxx - caozhanhao
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
#include "symxx/expr.hpp"
#include "symxx/num.hpp"
#include "symxx/utils.hpp"
#include "symxx/parser.hpp"
#include <iostream>
using namespace symxx;
using IntType = long long int;
int main()
{
  while (true)
  {
    try
    {
      std::string str;
      std::cout << ">> ";
      std::getline(std::cin, str);
      Parser<IntType> p{str};
      auto a = p.parse();
      std::cout << a << std::endl;
    }
    catch (Error &e)
    {
      std::cout << e.get_content() << std::endl;
    }
  }
  return 0;
}