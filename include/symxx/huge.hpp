//   Copyright 2022 symxx - caozhanhao
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

#ifndef SYMXX_HUGE_HPP
#define SYMXX_HUGE_HPP
#include "error.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <bitset>
#include <type_traits>
#include <tuple>
#include <bits/stl_algobase.h>
namespace symxx
{
  using digit = uint32_t;
  using sdigit = int32_t;
  using twodigits = uint64_t;
  using stwodigits = int64_t;
  constexpr digit SYMXX_HUGE_SHIFT = 30;
  constexpr digit SYMXX_HUGE_BASE = static_cast<digit>(1 << SYMXX_HUGE_SHIFT);
  constexpr digit SYMXX_HUGE_LOW_MASK = static_cast<digit>(SYMXX_HUGE_BASE - 1);
  constexpr digit SYMXX_HUGE_DECIMAL_SHIFT = 9;
  constexpr digit SYMXX_HUGE_DECIMAL_BASE = static_cast<digit>(1000000000);

  auto bit_length(digit d)
  {
    d |= 1;
#if (defined(__clang__) || defined(__GNUC__))
    return std::__lg(d) + 1;
#elif defined(_MSC_VER)
    unsigned long msb;
    _BitScanReverse(&msb, d);
    return (int)msb + 1;
#else
    digit k = 0;
    for (; d != 0; d >>= 1)
      ++k;
    return k;
#endif
  }
  // Shift the digits a[0,m] d bits left/right to z[0,m]
  // Returns the d bits shifted out of the top.
  digit digits_left_shift(std::vector<digit> &z, const std::vector<digit> &a, size_t m, int d)
  {
    digit carry = 0;
    for (size_t i = 0; i < m; ++i)
    {
      twodigits acc = static_cast<twodigits>(a[i]) << d | carry;
      z[i] = static_cast<digit>(acc) & SYMXX_HUGE_LOW_MASK;
      carry = static_cast<digit>(acc >> SYMXX_HUGE_SHIFT);
    }
    return carry;
  }
  digit digits_right_shift(std::vector<digit> &z, const std::vector<digit> &a, size_t m, int d)
  {
    digit carry = 0;
    digit mask = (static_cast<digit>(1) << d) - 1U;
    for (int i = m - 1; i >= 0; --i)
    {
      twodigits acc = static_cast<twodigits>(carry) << SYMXX_HUGE_SHIFT | a[i];
      carry = static_cast<digit>(acc) & mask;
      z[i] = static_cast<digit>(acc >> d);
    }
    return carry;
  }

  class Huge
  {
    friend std::ostream &operator<<(std::ostream &os, const Huge &i);

  private:
    std::vector<digit> digits;
    bool is_positive;

  public:
    Huge(std::vector<digit> s, bool p = true) : digits(std::move(s)), is_positive(p) {}
    Huge(const std::string &s)
    {
      if (s.empty())
        throw Error("Invaild string.");

      std::size_t pos = 0;
      is_positive = true;
      if (s[0] == '+')
        pos++;
      if (s[0] == '-')
      {
        is_positive = false;
        pos++;
      }
      constexpr size_t convwidth = 4;
      constexpr size_t convmultmax = 10000;
      const double sz = static_cast<double>(s.size() - pos) / std::log(32768) + 1;
      digits.reserve(static_cast<size_t>(sz));

      while (pos < s.size())
      {
        twodigits c = s[pos++] - '0';
        size_t i = 1;
        for (; i < convwidth && pos < s.size(); ++pos)
        {
          ++i;
          c = static_cast<twodigits>(c * 10 + s[pos] - '0');
        }

        twodigits convmult = convmultmax;

        if (i != convwidth) // the end of string
        {
          convmult = 10;
          for (; i > 1; --i)
            convmult *= 10;
        }

        // put c to digits
        for (auto &p : digits)
        {
          c += static_cast<twodigits>(p) * convmult;
          p = static_cast<digit>(c & SYMXX_HUGE_LOW_MASK);
          c >>= SYMXX_HUGE_SHIFT;
        }
        while (c != 0)
        {
          digits.emplace_back(static_cast<digit>(c & SYMXX_HUGE_LOW_MASK));
          c >>= SYMXX_HUGE_SHIFT;
        }
      }
    }
    Huge &operator+=(const Huge &h)
    {
      if (h.is_positive)
        internal_assign_add(h);
      else
        internal_assign_sub(h);
      return *this;
    }
    Huge operator+(const Huge &h) const
    {
      auto i = *this;
      i += h;
      return i;
    }
    Huge &operator-=(const Huge &h)
    {
      if (h.is_positive)
        internal_assign_sub(h);
      else
        internal_assign_add(h);
      return *this;
    }
    Huge operator-(const Huge &h) const
    {
      auto i = *this;
      i -= h;
      return i;
    }
    Huge &operator*=(const Huge &h)
    {
      if ((h.is_positive && is_positive) || (!h.is_positive && !is_positive))
        is_positive = true;
      else
        is_positive = false;
      internal_assign_mul(h);
      return *this;
    }
    Huge operator*(const Huge &h) const
    {
      auto i = *this;
      i *= h;
      return i;
    }
    Huge &operator/=(const Huge &h)
    {
      if (h.digits.size() == 0)
        throw Error("Huge can not be divided by zero.");
      if ((h.is_positive && is_positive) || (!h.is_positive && !is_positive))
        is_positive = true;
      else
        is_positive = false;
      *this = std::get<0>(internal_div(h));
      return *this;
    }
    Huge operator/(const Huge &h) const
    {
      return std::get<0>(internal_div(h));
    }
    Huge operator%(const Huge &h) const
    {
      return std::get<1>(internal_div(h));
    }
    template <typename T>
    T to() const
    {
      T result = 0;
      for (size_t i = 0; i < digits.size(); ++i)
      {
        result += digits[i] * std::pow(2, SYMXX_HUGE_SHIFT * i);
      }
      return result;
    }
    std::string to_string() const
    {
      std::vector<digit> out;
      for (auto rit = digits.crbegin(); rit < digits.crend(); ++rit)
      {
        auto hi = *rit;
        for (auto &j : out)
        {
          twodigits z = static_cast<twodigits>(j) << SYMXX_HUGE_SHIFT | hi;
          hi = static_cast<digit>(z / SYMXX_HUGE_DECIMAL_BASE);
          j = static_cast<digit>(z - static_cast<twodigits>(hi) * SYMXX_HUGE_DECIMAL_BASE);
        }
        while (hi != 0)
        {
          out.emplace_back(hi % SYMXX_HUGE_DECIMAL_BASE);
          hi /= SYMXX_HUGE_DECIMAL_BASE;
        }
      }

      if (out.empty()) // *this == 0
        out.emplace_back(0);

      // fill the string from right to left
      size_t len = (is_positive ? 0 : 1) + 1 + (out.size() - 1) * SYMXX_HUGE_DECIMAL_SHIFT;
      digit tenpow = 10;
      digit rem = out.back();
      while (rem >= tenpow)
      {
        tenpow *= 10;
        len++;
      }
      std::string ret(len, '\0');
      auto ret_it = ret.rbegin();

      for (auto it = out.cbegin(); it < out.cend() - 1; ++it)
      {
        rem = *it;
        for (size_t j = 0; j < SYMXX_HUGE_DECIMAL_SHIFT; ++j)
        {
          *(ret_it++) = '0' + rem % 10;
          rem /= 10;
        }
      }
      rem = out.back();
      do
      {
        *(ret_it++) = '0' + rem % 10;
        rem /= 10;
      } while (rem != 0);

      if (!is_positive)
        *(ret_it++) = '-';
      return ret;
    }

  private:
    void internal_assign_add(const Huge &h)
    {
      const std::vector<digit> &a = digits.size() > h.digits.size() ? digits : h.digits;
      const std::vector<digit> &b = digits.size() > h.digits.size() ? h.digits : digits;
      digit carry = 0;
      std::vector<digit> result;
      for (size_t i = 0; i < a.size(); ++i)
      {
        carry += a[i];
        if (i < b.size())
          carry += b[i];
        result.emplace_back(carry & SYMXX_HUGE_LOW_MASK);
        carry >>= SYMXX_HUGE_SHIFT;
      }
      if (carry != 0)
        result.emplace_back(carry);
      std::swap(result, digits);
    }
    void internal_assign_sub(const Huge &h)
    {
      const std::vector<digit> &a = digits.size() > h.digits.size() ? digits : h.digits;
      const std::vector<digit> &b = digits.size() > h.digits.size() ? h.digits : digits;
      std::vector<digit> result;
      if (digits.size() < h.digits.size())
      {
        is_positive = false;
      }
      else if (digits.size() == h.digits.size())
      {
        int i = static_cast<int>(digits.size());
        for (; --i >= 0 && digits[i] == h.digits[i];)
          ;
        if (i < 0)
        {
          digits.clear();
          return;
        }
        if (digits[i] < h.digits[i])
        {
          is_positive = false;
        }
      }
      digit borrow = 0;
      for (size_t i = 0; i < a.size(); ++i)
      {
        borrow = a[i] - borrow;
        if (i < b.size())
          borrow -= b[i];
        result.emplace_back(borrow & SYMXX_HUGE_LOW_MASK);
        borrow >>= SYMXX_HUGE_SHIFT;
        borrow &= 1;
      }
      std::swap(result, digits);
    }
    void internal_assign_mul(const Huge &h)
    {
      const std::vector<digit> &a = digits;
      const std::vector<digit> &b = h.digits;
      std::vector<digit> result(a.size() + b.size(), 0);
      for (size_t i = 0; i < a.size(); ++i)
      {
        twodigits carry = 0;
        auto itr = result.begin() + i;
        for (auto itb = b.cbegin(); itb < b.cend(); ++itb, ++itr)
        {
          carry += *itr + *itb * static_cast<twodigits>(a[i]);
          *itr = static_cast<digit>(carry & SYMXX_HUGE_LOW_MASK);
          carry >>= SYMXX_HUGE_SHIFT;
        }
        if (carry != 0)
          *itr += static_cast<digit>(carry & SYMXX_HUGE_LOW_MASK);
      }
      while (result.back() == 0)
        result.pop_back();
      std::swap(result, digits);
    }
    std::tuple<Huge, Huge> internal_div(const Huge &h) const
    {
      if (h.digits.size() == 1)
        return internal_div_by1(h.digits[0]);
      const std::vector<digit> &a = digits;
      const std::vector<digit> &b = h.digits;

      if (a.size() < b.size() || (a.size() == b.size() && a.back() < b.back())) //|a| < |b|
        return {Huge{std::vector<digit>()}, Huge{h.digits}};

      size_t sz_a = a.size();
      size_t sz_b = b.size();

      std::vector<digit> v(sz_a + 1, 0);
      std::vector<digit> w(sz_b, 0);
      int d = SYMXX_HUGE_SHIFT - bit_length(b.back());
      digits_left_shift(w, b, sz_b, d);
      digit carry = digits_left_shift(v, a, sz_a, d);
      if (carry != 0 || v[sz_a - 1] >= w[sz_b - 1])
      {
        v[sz_a] = carry;
        sz_a++;
      }

      size_t k = sz_a - sz_b;
      std::vector<digit> s(k, 0);
      digit wm1 = w[sz_b - 1];
      digit wm2 = w[sz_b - 2];
      for (auto vk = v.begin() + k, sk = s.begin() + k; vk-- > v.begin();)
      {
        digit vtop = *(vk + sz_b);
        twodigits vv = (static_cast<twodigits>(vtop) << SYMXX_HUGE_SHIFT) | *(vk + sz_b - 1);
        digit q = static_cast<digit>(vv / wm1);
        digit r = static_cast<digit>(vv % wm1);

        while (static_cast<twodigits>(wm2) * q >
               ((static_cast<twodigits>(r) << SYMXX_HUGE_SHIFT) | *(vk + sz_b - 2)))
        {
          --q;
          r += wm1;
          if (r >= SYMXX_HUGE_BASE)
            break;
        }
        sdigit zhi = static_cast<sdigit>(0);
        for (size_t i = 0; i < sz_b; ++i)
        {
          stwodigits z = static_cast<sdigit>(*(vk + i)) + zhi -
                         static_cast<stwodigits>(q) * static_cast<stwodigits>(w[i]);
          *(vk + i) = static_cast<digit>(z) & SYMXX_HUGE_LOW_MASK;
          // Since C++20, right shift on a signed number is definitely arithmetic right shift.
          // https://en.cppreference.com/w/cpp/language/operator_arithmetic
          zhi = static_cast<sdigit>(z >> SYMXX_HUGE_SHIFT);
        }
        if (static_cast<sdigit>(vtop) + zhi < 0)
        {
          carry = 0;
          for (size_t i = 0; i < sz_b; ++i)
          {
            carry += *(vk + i) + w[i];
            *(vk + i) = carry & SYMXX_HUGE_LOW_MASK;
            carry >>= SYMXX_HUGE_SHIFT;
          }
          --q;
        }
        *--sk = q;
      }
      // reuse w to store the rem
      digits_right_shift(w, v, sz_b, d);
      return {s, w};
    }
    std::tuple<Huge, Huge> internal_div_by1(digit b) const
    {
      digit rem = 0;
      std::vector<digit> s(digits.size(), 0);
      for (int i = digits.size() - 1; i >= 0; --i)
      {
        twodigits dividend = (static_cast<twodigits>(rem) << SYMXX_HUGE_SHIFT) | digits[i];
        digit quotient = static_cast<digit>(dividend / b);
        rem = dividend % b;
        s[i] = quotient;
      }
      return {s, Huge{std::vector<digit>{rem}}};
    }
  };
  std::ostream &operator<<(std::ostream &os, const Huge &i)
  {
    os << i.to_string();
    return os;
  }
}
#endif