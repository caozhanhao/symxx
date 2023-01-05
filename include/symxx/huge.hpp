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

#ifndef SYMXX_HUGE_HPP
#define SYMXX_HUGE_HPP

#include "error.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <type_traits>
#include <tuple>
#include <span>
#include <functional>
#include <memory>
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
  namespace huge_internal
  {
    namespace helper
    {
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
      digit digits_left_shift(const std::span<digit> z, const std::span<const digit> a, size_t m, int d)
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
      
      digit digits_right_shift(const std::span<digit> z, const std::span<const digit> a, size_t m, int d)
      {
        digit carry = 0;
        digit mask = (static_cast<digit>(1) << d) - 1U;
        for (auto i = static_cast<long long>(m - 1); i >= 0; --i)
        {
          twodigits acc = static_cast<twodigits>(carry) << SYMXX_HUGE_SHIFT | a[i];
          carry = static_cast<digit>(acc) & mask;
          z[i] = static_cast<digit>(acc >> d);
        }
        return carry;
      }
      
      std::tuple<std::vector<digit>, std::vector<digit>> k_mul_split(const std::span<const digit> n, const size_t &size)
      {
        std::vector<digit> low(std::min(n.size(), size));
        std::vector<digit> high(n.size() - low.size());
        std::copy(n.begin(), n.begin() + static_cast<long long>(low.size()), low.begin());
        std::copy(n.begin() + static_cast<long long>(low.size()),
                  n.begin() + static_cast<long long>( low.size()) + static_cast<long long>(high.size()), high.begin());
        return {std::move(high), std::move(low)};
      }
      
      //Requirements: x.size() >= y.size()
      digit digits_inplace_add(const std::span<digit> x, const std::span<const digit> y)
      {
        digit carry = 0;
        for (size_t i = 0; i < x.size(); ++i)
        {
          carry += x[i];
          if (i < y.size())
          {
            carry += y[i];
          }
          x[i] = carry & SYMXX_HUGE_LOW_MASK;
          carry >>= SYMXX_HUGE_SHIFT;
        }
        return carry;
      }
      
      digit digits_inplace_sub(const std::span<digit> x, const std::span<const digit> y)
      {
        digit borrow = 0;
        for (size_t i = 0; i < x.size(); ++i)
        {
          borrow = x[i] - borrow;
          if (i < y.size())
          {
            borrow -= y[i];
          }
          x[i] = borrow & SYMXX_HUGE_LOW_MASK;
          borrow >>= SYMXX_HUGE_SHIFT;
          borrow &= 1;
        }
        return borrow;
      }
    }
    
    void digits_add(const std::span<const digit> c, const std::span<const digit> d, std::vector<digit> &ret)
    {
      ret.clear();
      auto &a = c.size() > d.size() ? c : d;
      auto &b = c.size() > d.size() ? d : c;
      digit carry = 0;
      for (size_t i = 0; i < a.size(); ++i)
      {
        carry += a[i];
        if (i < b.size())
        {
          carry += b[i];
        }
        ret.emplace_back(carry & SYMXX_HUGE_LOW_MASK);
        carry >>= SYMXX_HUGE_SHIFT;
      }
      if (carry != 0)
      {
        ret.emplace_back(carry);
      }
    }
    
    void digits_sub(const std::span<const digit> a, const std::span<const digit> b, std::vector<digit> &ret)
    {
      //requirement a >= b
      ret.clear();
      digit borrow = 0;
      for (size_t i = 0; i < a.size(); ++i)
      {
        borrow = a[i] - borrow;
        if (i < b.size())
        {
          borrow -= b[i];
        }
        ret.emplace_back(borrow & SYMXX_HUGE_LOW_MASK);
        borrow >>= SYMXX_HUGE_SHIFT;
        borrow &= 1;
      }
    }
    
    void digits_simple_mul(const std::span<const digit> a, const std::span<const digit> b, std::vector<digit> &ret)
    {
      ret.clear();
      ret.resize(a.size() + b.size());
      for (size_t i = 0; i < a.size(); ++i)
      {
        twodigits carry = 0;
        auto itr = ret.begin() + static_cast<long long>(i);
        for (auto itb = b.begin(); itb < b.end(); ++itb, ++itr)
        {
          carry += *itr + *itb * static_cast<twodigits>(a[i]);
          *itr = static_cast<digit>(carry & SYMXX_HUGE_LOW_MASK);
          carry >>= SYMXX_HUGE_SHIFT;
        }
        if (carry != 0)
        {
          *itr += static_cast<digit>(carry & SYMXX_HUGE_LOW_MASK);
        }
      }
      while (ret.back() == 0)
      {
        ret.pop_back();
      }
    }
    
    //positive for a>b, 0 for a==b, negative for a<b
    int digits_cmp(const std::span<const digit> a, const std::span<const digit> b, bool apositive = true,
                   bool bpositive = true)
    {
      sdigit sign =
          (apositive ? 1 : -1) * static_cast<sdigit>(a.size()) - (bpositive ? 1 : -1) * static_cast<sdigit>(b.size());
      if (sign == 0)
      {
        sdigit diff = 0;
        for (auto i = static_cast<long long>(a.size()); --i >= 0;)
        {
          diff = static_cast<sdigit>(a[i]) - static_cast<sdigit>(b[i]);
          if (diff != 0)
          {
            break;
          }
        }
        sign = apositive ? diff : -diff;
      }
      return sign;
    }
    
    void digits_mul(const std::span<const digit> c, const std::span<const digit> d, std::vector<digit> &ret)
    {
      if (c.empty() || d.empty())
      {
        ret.clear();
        return;
      }
      ret.clear();
      auto &a = c.size() < d.size() ? c : d;
      auto &b = c.size() < d.size() ? d : c;
      constexpr digit cut_off = 70;
      constexpr digit square_cut_off = (2 * cut_off);
      bool eq = (digits_cmp(c, d) == 0);
      size_t i = eq ? square_cut_off : cut_off;
      if (a.size() <= i)
      {
        if (a.size() == 0)
          return;
        else
        {
          digits_simple_mul(a, b, ret);
          return;
        }
      }
      
      //if (2 * a.size() <= b.size())
      //  return k_lopsided_mul(a, b);
      size_t shift = b.size() >> 1;
      auto[ah, al] = helper::k_mul_split(a, shift);
      decltype(ah) bh, bl;
      if (eq)
      {
        bh = ah;
        bl = al;
      }
      else
      {
        auto s = helper::k_mul_split(b, shift);
        bh = std::get<0>(s);
        bl = std::get<1>(s);
      }
      ret.resize(a.size() + b.size());
      std::vector<digit> t1;
      digits_mul(ah, bh, t1);
      std::copy(t1.begin(), t1.end(), ret.begin() + shift * 2);
      
      std::vector<digit> t2;
      digits_mul(al, bl, t2);
      std::copy(t2.begin(), t2.end(), ret.begin());
      
      i = ret.size() - shift;
      helper::digits_inplace_sub({ret.begin() + shift, i}, t2);
      helper::digits_inplace_sub({ret.begin() + shift, i}, t1);
      digits_add(ah, al, t1);
      if (eq)
      {
        t2 = t1;
      }
      else
      {
        digits_add(bh, bl, t2);
      }
      
      std::vector<digit> t3;
      digits_mul(t1, t2, t3);
      helper::digits_inplace_add({ret.begin() + shift, i}, t3);
      while (ret.back() == 0)
      {
        ret.pop_back();
      }
    }
    
    void digits_divrem_by1(const std::span<const digit> c, digit b, std::vector<digit> &res, std::vector<digit> &rem)
    {
      digit remd = 0;
      res.resize(c.size());
      for (int i = c.size() - 1; i >= 0; --i)
      {
        twodigits dividend = (static_cast<twodigits>(remd) << SYMXX_HUGE_SHIFT) | c[i];
        digit quotient = static_cast<digit>(dividend / b);
        remd = dividend % b;
        res[i] = quotient;
      }
      if (remd != 0)
      {
        rem = {remd};
      }
    }
  
    void digits_divrem(const std::span<const digit> &a, const std::span<const digit> &b, std::vector<digit> &res,
                       std::vector<digit> &rem)
    {
      res.clear();
      rem.clear();
      int cmp = digits_cmp(a, b);
      if (cmp == 0)
      {
        res = {1};
        return;
      }
      else if (cmp < 0)
      {
        res.clear();
        rem.insert(rem.end(), a.begin(), a.end());
        return;
      }
      
      if (b.size() == 1)
      {
        digits_divrem_by1(a, b[0], res, rem);
        return;
      }
  
      size_t sz_a = a.size();
      size_t sz_b = b.size();
  
      std::vector<digit> v(sz_a + 1, 0);
      std::vector<digit> w(sz_b, 0);
      int d = SYMXX_HUGE_SHIFT - helper::bit_length(b.back());
      helper::digits_left_shift(w, b, sz_b, d);
      digit carry = helper::digits_left_shift(v, a, sz_a, d);
      if (carry != 0 || v[sz_a - 1] >= w[sz_b - 1])
      {
        v[sz_a] = carry;
        sz_a++;
      }
  
      size_t k = sz_a - sz_b;
      res.resize(k);
      digit wm1 = w[sz_b - 1];
      digit wm2 = w[sz_b - 2];
      for (auto vk = v.begin() + k, sk = res.begin() + k; vk-- > v.begin();)
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
          {
            break;
          }
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
      helper::digits_right_shift(w, v, sz_b, d);
      std::swap(rem, w);
      while (rem.back() == 0) rem.pop_back();
      while (res.back() == 0) res.pop_back();
    }
  
    // just for tests, unfinished
    void digits_rem(const std::span<const digit> &cd, const std::span<const digit> &dd, std::vector<digit> &rem)
    {
      //unfinished
      std::vector<digit> tmp;
      digits_divrem(cd, dd, tmp, rem);
    }
  
    void digits_gcd(const std::span<const digit> &a, const std::span<const digit> &b, std::vector<digit> &ret)
    {
      //unfinished
      if (b.empty())
      {
        ret.insert(ret.end(), a.begin(), a.end());
        return;
      }
      std::vector<digit> tmp;
      digits_rem(a, b, tmp);
      digits_gcd(b, tmp, ret);
    }
  
    void digits_pow(const std::span<const digit> &a, const std::span<const digit> &b, std::vector<digit> &ret)
    {
      ret.clear();
      if (b.empty())//b==0
      {
        ret.emplace_back(1);
        return;
      }
      else if (a.empty())//a==0
      {
        return;
      }
      else if (b.size() == 1 && b[0] == 1)//b==1
      {
        ret.insert(ret.end(), b.begin(), b.end());
        return;
      }
      else if (a.size() == 1 && a[0] == 1)//a==1
      {
        ret.emplace_back(1);
        return;
      }
      //unfinished
      throw Error("Unfinished");
    }
  
    void digits_bitwise(const std::span<const digit> &a, const std::span<const digit> &b,
                        const std::function<digit(digit, digit)> &process)
    {
    
    }
  
    //
  
  
    template<typename U>
    std::enable_if_t<std::is_integral_v<std::decay_t<U>>>
    digits_from_int(const U &val, std::vector<digit> &digits)
    {
      for (U u = val > 0 ? val : static_cast<U>(0) - val;
           u != 0; u >>= static_cast<U>(SYMXX_HUGE_SHIFT))
      {
        digits.emplace_back(static_cast<digit>(u & SYMXX_HUGE_LOW_MASK));
      }
    }
  
    struct IntTag {};
    struct FloatingTag {};
    template<typename T>
    struct TagDispatch
    {
      using tag = std::conditional_t<std::is_integral_v<std::decay_t<T>>, IntTag, FloatingTag>;
    };
  }
  class Huge
  {
    friend std::ostream &operator<<(std::ostream &os, const Huge &i);
  
    friend std::tuple<Huge, Huge> divrem(const Huge &h1, const Huge &h2);

  private:
    std::vector<digit> digits;
    bool is_positive;

  public:
    Huge() : is_positive(true) {}
  
    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<U>>>>
    Huge(U val): is_positive(val >= 0)
    {
      using Ut = std::decay_t<U>;
      if constexpr(std::is_unsigned_v<Ut>)
      {
        if constexpr(std::is_integral_v<Ut>)
        {
          huge_internal::digits_from_int(std::forward<U>(val), digits);
        }
        else
        {
          huge_internal::digits_from_int(static_cast<unsigned long long>(std::forward<U>(val)), digits);
        }
      }
      else
      {
        if constexpr(std::is_integral_v<Ut>)
        {
          huge_internal::digits_from_int(std::abs(val), digits);
        }
        else
        {
          huge_internal::digits_from_int(static_cast<unsigned long long>(std::abs(val)), digits);
        }
      }
    }
  
    explicit operator int() const { return to<int>(); }
  
    explicit operator unsigned int() const { return to<unsigned int>(); }
  
    explicit operator long() const { return to<long>(); }
  
    explicit operator unsigned long() const { return to<unsigned long>(); }
  
    explicit operator long long() const { return to<long long>(); }
  
    explicit operator unsigned long long() const { return to<unsigned long long>(); }
  
    explicit operator double() const { return to<double>(); }
  
    explicit operator long double() const { return to<long double>(); }
  
    explicit Huge(std::vector<digit> s, bool p = true) : digits(std::move(s)), is_positive(p) {}
  
    explicit Huge(std::initializer_list<digit> s, bool p = true) : digits(std::move(s)), is_positive(p) {}
  
    Huge(const std::string &s)
    {
      std::size_t pos = 0;
      std::size_t end = s.size();
      while (!(isdigit(s[pos]) || s[pos] == '+' || s[pos] == '-') && pos < end) ++pos;
      if (pos >= end) throw Error("Invaild string.");
      is_positive = true;
      if (s[0] == '+')
      {
        pos++;
      }
      if (s[0] == '-')
      {
        is_positive = false;
        pos++;
      }
      constexpr size_t convwidth = 9;
      constexpr size_t convmultmax = 1000000000;
      const double sz = static_cast<double>(s.size() - pos) * std::log(10) / std::log(32768) + 1;
      digits.reserve(static_cast<size_t>(sz));
    
      while (pos < end)
      {
        if (!isdigit(s[pos])) break;
        twodigits c = s[pos++] - '0';
        size_t i = 1;
        for (; i < convwidth && pos < end; ++pos)
        {
          if (!isdigit(s[pos]))
          {
            end = pos;
          }
          else
          {
            c = static_cast<twodigits>(c * 10 + s[pos] - '0');
            ++i;
          }
        }
      
        twodigits convmult = convmultmax;
      
        if (i != convwidth) // the end of string
        {
          convmult = 10;
          for (; i > 1; --i)
            convmult *= 10;
        }
      
        // put c to digits
        for (auto &p: digits)
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
      std::vector<digit> tmp;
      if ((h.is_positive && is_positive) || (!h.is_positive && !is_positive))
      {
        huge_internal::digits_add(digits, h.digits, tmp);
      }
      else
      {
        int cmp = huge_internal::digits_cmp(digits, h.digits);
        if (cmp == 0)
        {
          digits.clear();
          return *this;
        }
        else if (cmp < 0)
        {
          is_positive = !is_positive;
          huge_internal::digits_sub(h.digits, digits, tmp);
        }
        else
        {
          huge_internal::digits_sub(digits, h.digits, tmp);
        }
      }
      std::swap(tmp, digits);
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
      std::vector<digit> tmp;
      int cmp = huge_internal::digits_cmp(digits, h.digits);
      if (cmp == 0)
      {
        digits.clear();
        return *this;
      }
      if ((h.is_positive && !is_positive) || (!h.is_positive && is_positive))
      {
        huge_internal::digits_add(digits, h.digits, tmp);
      }
      else
      {
        if (cmp < 0)
        {
          is_positive = !is_positive;
          huge_internal::digits_sub(h.digits, digits, tmp);
        }
        else
        {
          huge_internal::digits_sub(digits, h.digits, tmp);
        }
      }
      std::swap(tmp, digits);
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
      std::vector<digit> tmp;
      is_positive = ((h.is_positive && is_positive) || (!h.is_positive && !is_positive));
      huge_internal::digits_mul(digits, h.digits, tmp);
      std::swap(tmp, digits);
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
      if (h.digits.empty()) throw Error("Huge can not be divided by zero.");
      is_positive = ((h.is_positive && is_positive) || (!h.is_positive && !is_positive));
      std::vector<digit> res;
      std::vector<digit> rem;
      huge_internal::digits_divrem(digits, h.digits, res, rem);
      std::swap(res, digits);
      return *this;
    }
  
    Huge operator/(const Huge &h) const
    {
      auto i = *this;
      i /= h;
      return i;
    }
  
    Huge operator-() const
    {
      return Huge(digits, !is_positive);
    }
  
    Huge &operator++()
    {
      *this += 1;
      return *this;
    }
  
    Huge operator++(int)
    {
      auto tmp = *this;
      *this += 1;
      return tmp;
    }
  
    Huge &operator--()
    {
      *this -= 1;
      return *this;
    }
  
    Huge operator--(int)
    {
      auto tmp = *this;
      *this -= 1;
      return tmp;
    }
  
    Huge &operator%=(const Huge &h)
    {
      std::vector<digit> rem;
      huge_internal::digits_rem(digits, h.digits, rem);
      std::swap(rem, digits);
      return *this;
    }
  
    Huge operator%(const Huge &h) const
    {
      auto i = *this;
      i %= h;
      return i;
    }
  
    //TODO not finished
    [[nodiscard]] Huge gcd(const Huge &h) const
    {
      std::vector<digit> ret;
      huge_internal::digits_gcd(digits, h.digits, ret);
      return Huge(ret);
    }
  
    [[nodiscard]] Huge pow(const Huge &h) const
    {
      std::vector<digit> ret;
      huge_internal::digits_pow(digits, h.digits, ret);
      return Huge(ret);
    }
  
    //
    [[nodiscard]] Huge abs() const
    {
      return Huge(digits, true);
    }
  
    bool operator<(const Huge &h) const
    {
      return huge_internal::digits_cmp(digits, h.digits, is_positive, h.is_positive) < 0;
    }
  
    bool operator>(const Huge &h) const
    {
      return huge_internal::digits_cmp(digits, h.digits, is_positive, h.is_positive) > 0;
    }
  
    bool operator==(const Huge &h) const
    {
      return huge_internal::digits_cmp(digits, h.digits, is_positive, h.is_positive) == 0;
    }
  
    bool operator!=(const Huge &h) const
    {
      return huge_internal::digits_cmp(digits, h.digits, is_positive, h.is_positive) != 0;
    }
  
    bool operator<=(const Huge &h) const
    {
      return huge_internal::digits_cmp(digits, h.digits, is_positive, h.is_positive) <= 0;
    }
  
    bool operator>=(const Huge &h) const
    {
      return huge_internal::digits_cmp(digits, h.digits, is_positive, h.is_positive) >= 0;
    }
  
    [[nodiscard]] std::string to_string() const
    {
      std::vector<digit> out;
      for (auto rit = digits.crbegin(); rit < digits.crend(); ++rit)
      {
        auto hi = *rit;
        for (auto &j: out)
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
      {
        *(ret_it++) = '-';
      }
      return ret;
    }
  
    template<typename U>
    [[nodiscard]] std::enable_if_t<std::is_arithmetic_v<std::decay_t<U>>, U>
    to() const
    {
      return internal_to<U>(typename huge_internal::TagDispatch<std::decay_t<U>>::tag{});
    }

  private:
    template<typename U>
    [[nodiscard]] U internal_to(huge_internal::IntTag) const
    {
      if (digits.empty()) return static_cast<U>(0);
      if (!is_positive && std::is_unsigned_v<std::decay_t<U>>)
      {
        throw Error("The Huge is negative but U is unsigned.[" + to_string() + "].");
      }
      if (digits.size() == 1
          && digits[0] <= std::numeric_limits<U>::max()
          && static_cast<sdigit>(digits[0]) >= std::numeric_limits<U>::lowest())
      {
        return static_cast<U>(digits[0]) * static_cast<U>(static_cast<int>(is_positive ? 1 : -1));
      }
      else
      {
        if ((digits.size() - 1) * SYMXX_HUGE_SHIFT > sizeof(U) * 8)
        {
          throw Error("The Huge is too big.[" + to_string() + "].");
        }
        U result = 0;
        for (auto rit = digits.crbegin(); rit < digits.crend(); ++rit)
        {
          result |= static_cast<twodigits>(*rit);
          if (rit < digits.crend() - 1)
          {
            result <<= SYMXX_HUGE_SHIFT;
          }
        }
        return result * static_cast<U>(static_cast<int>((is_positive ? 1 : -1)));
      }
      return 0;
    }
  
    template<typename U>
    [[nodiscard]] U internal_to(huge_internal::FloatingTag) const
    {
      auto tmp = abs().to<unsigned long long>();
      if (tmp > std::numeric_limits<U>::max())
      {
        throw Error("The Huge is too big.[" + to_string() + "].");
      }
      return static_cast<U>(tmp) * (is_positive ? 1 : -1);
    }
  };
  
  std::ostream &operator<<(std::ostream &os, const Huge &i)
  {
    os << i.to_string();
    return os;
  }
  
  std::tuple<Huge, Huge> divrem(const Huge &h1, const Huge &h2)
  {
    if (h2.digits.size() == 0)
    {
      throw Error("Huge can not be divided by zero.");
    }
    bool is_positive;
    is_positive = ((h1.is_positive && h2.is_positive) || (!h1.is_positive && !h2.is_positive));
    std::vector<digit> res;
    std::vector<digit> rem;
    huge_internal::digits_divrem(h1.digits, h2.digits, res, rem);
    return {Huge{res, is_positive}, Huge{rem}};
  }
}
#endif