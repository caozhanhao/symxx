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
#ifndef SYMXX_NUM_HPP
#define SYMXX_NUM_HPP
#include "error.hpp"
#include <cmath>
#include <limits>
#include <numeric>
#include <ostream>
#include <string>
#include <type_traits>
namespace symxx
{
  template <typename T,
            typename = typename std::enable_if_t<std::is_integral<T>::value>>
  inline T gcd(T a, T b)
  {
    return b > 0 ? gcd(b, a % b) : a;
  }
  template <typename T,
            typename = typename std::enable_if_t<std::is_integral<T>::value>>
  class Rational
  {
    template <typename U>
    friend std::ostream &operator<<(std::ostream &os, const Rational<U> &i);

  private:
    T numerator;
    T denominator;

  public:
    Rational(T n)
        : numerator(n), denominator(1)
    {
    }
    Rational(T n, T d)
        : numerator(n), denominator(d)
    {
      if (denominator == 0)
        throw Error(SYMXX_ERROR_LOCATION, __func__, "denominator must not be 0.");
      reduct();
    }
    Rational()
        : numerator(0), denominator(1)
    {
    }
    Rational(const std::string &n)
    {
      auto k = n.find('/');
      if (k == std::string::npos)
      {
        numerator = std::stoll(n);
        denominator = 1;
      }
      else
      {
        numerator = std::stoll(n.substr(0, k));
        denominator = std::stoll(n.substr(k + 1));
      }
      if (denominator == 0)
        throw Error(SYMXX_ERROR_LOCATION, __func__, "denominator must not be 0.");
    }
    Rational operator+(const Rational &i) const
    {
      auto a = *this;
      a += i;
      return a;
    }
    Rational &operator+=(const Rational &i)
    {
      numerator = numerator * i.denominator + i.numerator * denominator;
      denominator *= i.denominator;
      if (denominator == 0)
        throw Error(SYMXX_ERROR_LOCATION, __func__, "denominator must not be 0.");
      return *this;
    }
    Rational operator-(const Rational &i) const
    {
      auto a = *this;
      a -= i;
      return a;
    }
    Rational &operator-=(const Rational &i)
    {
      *this += i.opposite();
      return *this;
    }
    Rational operator*(const Rational &i) const
    {
      auto a = *this;
      a *= i;
      return a;
    }
    Rational &operator*=(const Rational &i)
    {
      numerator *= i.numerator;
      denominator *= i.denominator;
      if (denominator == 0)
        throw Error(SYMXX_ERROR_LOCATION, __func__, "denominator must not be 0.");
      return *this;
    }
    Rational operator/(const Rational &i) const
    {
      auto a = *this;
      a /= i;
      return a;
    }
    Rational &operator/=(const Rational &i)
    {
      *this *= i.reciprocate();
      return *this;
    }
    Rational operator^(std::make_unsigned_t<T> p) const
    {
      auto a = *this;
      a ^= p;
      return a;
    }
    Rational &operator^=(std::make_unsigned_t<T> p)
    {
      numerator = static_cast<T>(std::pow(numerator, p));
      denominator = static_cast<T>(std::pow(denominator, p));
      if (denominator == 0)
        throw Error(SYMXX_ERROR_LOCATION, __func__, "denominator must not be 0.");
      return *this;
    }
    bool operator<(const Rational &r) const
    {
      return numerator * r.denominator < r.numerator * denominator;
    }
    bool operator==(const Rational &r) const
    {
      return numerator == r.numerator && denominator == r.denominator;
    }
    bool operator!=(const Rational &r) const { return !(*this == r); }
    bool operator>(const Rational &r) const
    {
      return numerator * r.denominator > r.numerator * denominator;
    }
    bool is_int() const { return numerator % denominator == 0; }
    Rational opposite() const { return {-numerator, denominator}; }
    Rational reciprocate() const { return {denominator, numerator}; }
    void reduct()
    {
      T g = gcd(std::abs(numerator), std::abs(denominator));
      numerator /= g;
      denominator /= g;
      if (denominator == -1)
      {
        numerator = -numerator;
        denominator = 1;
      }
    }
    T get_numerator() const { return numerator; }
    T get_denominator() const { return denominator; }
    template <typename U>
    U to() const
    {
      return static_cast<U>(numerator) / static_cast<U>(denominator);
    }
    T to_t() const
    {
      return numerator / denominator;
    }
  };
  template <typename U>
  std::ostream &
  operator<<(std::ostream &os, const Rational<U> &i)
  {
    if (i.denominator != 1)
      os << "(" << i.numerator << "/" << i.denominator << ")";
    else
      os << i.numerator;
    return os;
  }
}
#endif