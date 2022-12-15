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
    std::string stra, strb, strc;
    Rational<T> a, b, c;
    while (true)
    {
      std::cout << "Input a,b,c: ";
      std::cin >> stra >> strb >> strc;
      try
      {
        a = Rational<T>{stra};
        b = Rational<T>{strb};
        c = Rational<T>{strc};
      }
      catch (...)
      {
        std::cout << "Please input three numbers(includes fraction)." << std::endl;
      }
      Rational<T> delta = (b ^ 2) - (a * c * 4);
      Term<T> bt{b.opposite()};
      Poly<T> bp{bt};
      Poly<T> a2{Term<T>{a * 2}};
      Poly<T> gdelta = {Term<T>{nth_root(2, delta)}};
      Frac<T> x1 = {bp + gdelta, a2};
      Frac<T> x2 = {bp - gdelta, a2};

      Term<T> eqa{a, {"x", "x"}};
      Term<T> eqb{b, {"x"}};
      Term<T> eqc{c};
      Poly<T> eq{eqa, eqb, eqc};
      std::cout << "The root of " << eq
                << " =  0:\n"
                << "x1 = " << x1 << ",x2 = " << x2 << std::endl;
    }
  }
  catch (Error &e)
  {
    std::cout << e.get_content() << std::endl;
  }
  return 0;
}