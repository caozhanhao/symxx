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
#include "expr.hpp"
#include <iostream>
using namespace symxx;
using T = long long int;
int main()
{
  try
  {
    Term<T> x{1, {"x"}};
    Term<T> x2{1, {"x", "x"}};
    Poly<T> num{{Term<T>{1}}};
    Poly<T> den{{x, x2}};
    Frac<T> func{num, den};
    std::cout << "f(x) = " << func << std::endl;
    std::string input;
    Rational<T> i;
    while (true)
    {
      std::cout << ">> ";
      std::cin >> input;
      try
      {
        i = Rational<T>{input};
      }
      catch (...)
      {
        if (input == "quit")
          return 0;

        std::cout << "Input Error." << std::endl;
        continue;
      }
      std::cout << "f(" << i << ") = " << func.set_var({{"x", i}}) << std::endl;
    }
  }
  catch (Error& err)
  {
    std::cout << err.get_content() << std::endl;
  }
  return 0;
}