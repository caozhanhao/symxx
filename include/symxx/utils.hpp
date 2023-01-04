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

#include "huge.hpp"
#include "error.hpp"
#include <utility>
#include <cmath>
#include <numeric>
#include <string>
#include <type_traits>

namespace symxx::utils
{
  template<typename T>
  inline T To_int(const std::string &num)
  {
    return std::stoll(num);
  }
  
  template<>
  inline Huge To_int(const std::string &num)
  {
    return Huge{num};
  }
  
  template<typename T>
  inline T Abs(const T &num)
  {
    return std::abs(num);
  }
  
  template<>
  inline Huge Abs(const Huge &num)
  {
    return num.abs();
  }
  
  template<typename T, typename U>
  inline auto Pow(const T &num, U &&power)
  {
    return std::pow(num, std::forward<U>(power));
  }
  
  template<typename U>
  inline auto Pow(const symxx::Huge &num, U &&power)
  {
    return num.pow(std::forward<U>(power));
  }
  
  template<typename T, typename U>
  inline auto Gcd(const T &a, U &&b)
  {
    return std::gcd(a, std::forward<U>(b));
  }
  
  template<typename U>
  inline auto Gcd(const symxx::Huge &a, U &&b)
  {
    return a.gcd(std::forward<U>(b));
  }
  
  
  template<typename T>
  class Make_unsigned
  {
  public:
    using type = std::make_unsigned_t<T>;
  };
  
  template<>
  class Make_unsigned<::symxx::Huge>
  {
  public:
    using type = ::symxx::Huge;
  };
  
  template<typename T>
  using Make_unsigned_t = typename Make_unsigned<T>::type;
}
#endif