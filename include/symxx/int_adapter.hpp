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

// This is a large integer class that borrows its ideas from CPython's int
// https://github.com/python/cpython/blob/main/Objects/longobject.c

#ifndef SYMXX_INT_ADAPTER_HPP
#define SYMXX_INT_ADAPTER_HPP

#include "huge.hpp"
#include "error.hpp"
#include <utility>
#include <cmath>
#include <numeric>
#include <string>
#include <type_traits>

namespace symxx
{
  template<typename T>
  inline std::string To_String(const T &num)
  {
    return std::to_string(num);
  }
  
  template<typename T>
  inline T To_int(const std::string &num)
  {
    return std::stoll(num);
  }
  
  template<typename T>
  inline T Abs(const T &num)
  {
    return std::abs(num);
  }
  
  template<typename T, typename U>
  inline auto Div(const T &num, const U &d)
  {
    return std::div(num, d);
  }
  
  
  template<typename T, typename U>
  inline auto Pow(const T &num, U &&power)
  {
    return std::pow(num, std::forward<U>(power));
  }
  
  template<typename T, typename U>
  inline auto Gcd(const T &a, U &&b)
  {
    return std::gcd(a, std::forward<U>(b));
  }
  
  template<typename T>
  class Make_unsigned
  {
  public:
    using type = std::make_unsigned_t<T>;
  };
  
  template<typename T>
  using Make_unsigned_t = typename Make_unsigned<T>::type;
#if defined(SYMXX_ENABLE_HUGE)
  template<>
  inline Huge To_int(const std::string &num)
  {
    return Huge{num};
  }
    template<>
  inline std::string To_String(const Huge &num)
  {
    return num.to_string();
  }
  template<>
  class Make_unsigned<::symxx::Huge>
  {
  public:
    using type = ::symxx::Huge;
  };
  template<typename U>
  inline auto Gcd(const symxx::Huge &a, U &&b)
  {
    return a.gcd(std::forward<U>(b));
  }
  inline auto Div(const symxx::Huge &num, const symxx::Huge &d)
  {
    return divrem(num, d);
  }
  template<>
  inline Huge Abs(const Huge &num)
  {
    return num.abs();
  }
  template<typename U>
  inline auto Pow(const symxx::Huge &num, U &&power)
  {
    return num.pow(std::forward<U>(power));
  }
#endif
#if defined(SYMXX_ENABLE_INT128)
  constexpr double SYMXX_INT_ADAPTER_LOG2_10 = 3.32192809488736234;
  inline auto Bit_length(const __int128_t& d)
  {
    size_t k = 0;
    __int128_t tmp = std::abs(d);
    if(tmp < 0)
      throw Error("Unexpected overflow.");
    for (;tmp != 0; tmp >>= 1)
      ++k;
    return k;
  }
  
  template<>
  inline std::string To_String(const __int128_t &num)
  {
    std::string ret;
    auto sz = static_cast<size_t>(static_cast<double>(Bit_length(num)) / SYMXX_INT_ADAPTER_LOG2_10) + 1;
    if(num < 0) sz++;
    ret.resize(sz);
    auto rit = ret.rbegin();
    for (auto rem = num; rem > 0; rem /= 10)
      *rit++ = '0' + static_cast<int>(rem % 10);
    if(num < 0) ret[0] = '-';
    if(*ret.begin() == '\0') ret.erase(ret.begin());
    return ret;
  }
  
  template<>
  inline std::string To_String(const std::make_unsigned_t<__int128_t> &num)
  {
    std::string ret;
    auto sz = static_cast<size_t>(static_cast<double>(Bit_length(num)) / SYMXX_INT_ADAPTER_LOG2_10) + 1;
    ret.resize(sz);
    auto rit = ret.rbegin();
    for (auto rem = num; rem > 0; rem /= 10)
      *rit++ = '0' + static_cast<int>(rem % 10);
    if(*ret.begin() == '\0') ret.erase(ret.begin());
    return ret;
  }
#endif
}
#endif