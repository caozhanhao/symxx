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
#include <string>
#include <type_traits>

namespace symxx
{
  template<typename T>
  inline std::string adapter_to_string(const T &num)
  {
    return std::to_string(num);
  }
  
  template<typename T>
  inline T adapter_to_int(const std::string &num)
  {
    return std::stoll(num);
  }
  
  template<typename T>
  inline auto adapter_abs(const T &num)
  {
    return std::abs(num);
  }
  
  template<typename T, typename U>
  inline auto adapter_div(const T &num, const U &d)
  {
    return std::div(num, d);
  }
  
  template<typename T, typename U>
  inline double adapter_pow(const T &num, U &&power)
  {
    return std::pow(num, std::forward<U>(power));
  }
  
  //adapted from:
  //https://stackoverflow.com/questions/12168348/ways-to-do-modulo-multiplication-with-primitive-types
  template<typename T>
  T adapter_mulmod(T a, T b, T m)
  {
    T res = 0;
    while (a != 0)
    {
      if (a & 1) res = (res + b) % m;
      a >>= 1;
      b = (b << 1) % m;
    }
    return res;
  }
  
  //adapted from:
  //https://stackoverflow.com/questions/8496182/calculating-powa-b-mod-n/8498251#8498251
  template<typename T>
  T adapter_modpow(T base, T exp, T modulus)
  {
    base %= modulus;
    T result = 1;
    while (exp > 0)
    {
      if (exp & 1) result = (result * base) % modulus;
      base = (base * base) % modulus;
      exp >>= 1;
    }
    return result;
  }
  
  template<typename T, typename U>
  inline auto adapter_gcd(const T &a, U &&b)
  {
    return std::gcd(a, std::forward<U>(b));
  }
  
  template<typename T, typename U>
  inline auto adapter_lcm(const T &a, U &&b)
  {
    return std::lcm(a, std::forward<U>(b));
  }
  
  template<typename T>
  inline auto adapter_sqrt(const T &a)
  {
    return std::sqrt(a);
  }
  
  template<typename T>
  inline auto adapter_log(const T &a)
  {
    return std::log(a);
  }
  
  template<typename T>
  class adapter_make_unsigned
  {
  public:
    using type = std::make_unsigned_t<T>;
  };
  
  template<typename T>
  using Make_unsigned_t = typename adapter_make_unsigned<T>::type;
#if defined(SYMXX_ENABLE_HUGE)
  template<>
  inline Huge adapter_to_int(const std::string &num)
  {
    return Huge{num};
  }
  template<>
  inline std::string adapter_to_string(const Huge &num)
  {
    return num.to_string();
  }
  template<>
  class adapter_make_unsigned<::symxx::Huge>
  {
  public:
    using type = ::symxx::Huge;
  };
  template<typename U>
  inline auto adapter_gcd(const symxx::Huge &a, U &&b)
  {
    return a.gcd(std::forward<U>(b));
  }
  inline auto adapter_div(const symxx::Huge &num, const symxx::Huge &d)
  {
    return divrem(num, d);
  }
  template<>
  inline Huge adapter_abs(const Huge &num)
  {
    return num.abs();
  }
  template<typename U>
  inline auto adapter_pow(const symxx::Huge &num, U &&power)
  {
    return num.pow(std::forward<U>(power));
  }
#endif
#if defined(SYMXX_ENABLE_INT128)
  template<>
  class adapter_make_unsigned<__int128_t>
  {
  public:
    using type = __uint128_t;
  };
  
  constexpr double SYMXX_INT_ADAPTER_LOG2_10 = 3.32192809488736234;
  
  inline auto adapter_bit_width(const __int128_t &d)
  {
    size_t k = 0;
    __int128_t tmp = std::abs(d);
    if (tmp < 0)
    {
      throw Error("Unexpected overflow.");
    }
    for (; tmp != 0; tmp >>= 1)
    {
      ++k;
    }
    return k;
  }
  
  template<>
  inline std::string adapter_to_string(const __int128_t &num)
  {
    std::string ret;
    auto sz = static_cast<size_t>(static_cast<double>(adapter_bit_width(num)) / SYMXX_INT_ADAPTER_LOG2_10) + 1;
    if (num < 0) sz++;
    ret.resize(sz);
    auto rit = ret.rbegin();
    for (auto rem = adapter_abs(num); rem > 0; rem /= 10)
    {
      *rit++ = '0' + static_cast<int>(rem % 10);
    }
    if(num < 0) ret[0] = '-';
    if(*ret.begin() == '\0') ret.erase(ret.begin());
    return ret;
  }
  
  template<>
  inline std::string adapter_to_string(const Make_unsigned_t<__int128_t> &num)
  {
    std::string ret;
    auto sz = static_cast<size_t>(static_cast<double>(adapter_bit_width(num)) / SYMXX_INT_ADAPTER_LOG2_10) + 1;
    ret.resize(sz);
    auto rit = ret.rbegin();
    for (auto rem = num; rem > 0; rem /= 10)
    {
      *rit++ = '0' + static_cast<int>(rem % 10);
    }
    if (*ret.begin() == '\0') ret.erase(ret.begin());
    return ret;
  }
  
  inline auto adapter_abs(const __int128_t &num)
  {
    return num > 0 ? num : (static_cast<__int128_t>(0) - num);
  }
  
  inline auto adapter_abs(const Make_unsigned_t<__int128_t> &num)
  {
    return num;
  }
  
  inline __int128_t adapter_to_int128(const std::string &s)
  {
    __int128_t m = 0;
    for (size_t i = 0; i < s.size(); i++)
    {
      m *= 10;
      m += s[i] - 48;
    }
    return m;
  }
  
  inline __uint128_t adapter_to_uint128(const std::string &s)
  {
    __uint128_t m = 0;
    for (size_t i = 0; i < s.size(); i++)
    {
      m *= 10;
      m += s[i] - 48;
    }
    return m;
  }
  
  template<>
  inline __int128_t adapter_to_int(const std::string &num)
  {
    return adapter_to_int128(num);
  }

#endif
}
#endif