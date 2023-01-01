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
#ifndef SYMXX_UTILS_HPP
#define SYMXX_UTILS_HPP
#include "num.hpp"
#include "frac.hpp"
#include "error.hpp"
#include <utility>
#include <map>
#include <string>
namespace symxx
{
  template<typename T>
  std::pair<Frac < T>, Frac <T>>
  
  solve_quadratic(const Rational <T> &a, const Rational <T> &b, const Rational <T> &c)
  {
    Rational<T> delta = (b ^ 2) - (a * c * 4);
    Frac<T> gdelta{nth_root(2, delta)};
    auto x1 = (Frac<T>{b.negate()} + gdelta) / Frac<T>{a * 2};
    auto x2 = (Frac<T>{b.negate()} - gdelta) / Frac<T>{a * 2};
    return {x1, x2};
  }
}
#endif