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
#include <stdexcept>
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
      reduce();
    }
    Rational()
        : numerator(0), denominator(1)
    {
    }
    Rational(const std::string &n)
    {
      try
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
      }
      catch (std::invalid_argument &)
      {
        throw Error(SYMXX_ERROR_LOCATION, __func__, "Invaild string.");
      }
      catch (std::out_of_range &)
      {
        throw Error(SYMXX_ERROR_LOCATION, __func__, "The number is out of range.");
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
      reduce();
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
      reduce();
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
      reduce();
      return *this;
    }
    Rational operator^(const Rational<T> &p) const
    {
      auto a = *this;
      a ^= p;
      return a;
    }
    Rational &operator^=(const Rational<T> &p)
    {
      numerator = static_cast<T>(std::pow(numerator, p.template to<double>()));
      denominator = static_cast<T>(std::pow(denominator, p.template to<double>()));
      if (denominator == 0)
        throw Error(SYMXX_ERROR_LOCATION, __func__, "denominator must not be 0.");
      reduce();
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
    bool is_square() const
    {
      return (std::sqrt(std::abs(numerator)) - static_cast<T>(std::sqrt(std::abs(numerator))) == 0) &&
             (std::sqrt(std::abs(denominator)) - static_cast<T>(std::sqrt(std::abs(denominator))) == 0);
    }
    Rational opposite() const
    {
      return {-numerator, denominator};
    }
    Rational reciprocate() const { return {denominator, numerator}; }
    void reduce()
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
  template <typename T,
            typename = typename std::enable_if_t<std::is_integral<T>::value>>
  bool is_prime(T a)
  {
    T i = 2;
    while (i < a)
    {
      if (a % i == 0)
        break;
      i++;
    }
    if (i == a)
      return true;
    else
      return false;
  }

  template <typename T>
  class Real
  {
    template <typename U>
    friend std::ostream &operator<<(std::ostream &os, const Real<U> &i);

  private:
    using IndexT = std::make_unsigned_t<T>;
    IndexT index;
    Rational<T> radicand;
    Rational<T> coe;

  public:
    Real(const Rational<T> &c, const Rational<T> &r, IndexT i)
        : index(i), radicand(r), coe(c) { reduce(); }
    Real(const Rational<T> &c)
        : index(1), radicand(1), coe(c) { reduce(); }
    Real(const T &c)
        : index(1), radicand(1), coe(c) { reduce(); }
    Real() : index(1), radicand(1), coe(0) {}
    bool is_equivalent_with(const Real &t) const
    {
      if (radicand == t.radicand && radicand == 1)
        return true;
      if (index == t.index && index == 1)
        return true;
      return (radicand == t.radicand && index == t.index);
    }
    Real &operator+=(const Real &r)
    {
      if (!is_equivalent_with(r))
        throw Error(SYMXX_ERROR_LOCATION, __func__, "radicand and index must be the same.");
      coe += r.coe;
      reduce();
      return *this;
    }
    Real operator+(const Real &r) const
    {
      auto a = *this;
      a += r;
      return a;
    }

    Real &operator-=(const Real &r)
    {
      *this += r.opposite();
      reduce();
      return *this;
    }
    Real operator-(const Real &r) const
    {
      auto a = *this;
      a -= r;
      return a;
    }

    Real &operator*=(const Real &r)
    {
      radicand ^= r.index;
      radicand *= r.radicand ^ index;
      index *= r.index;
      coe *= r.coe;
      reduce();
      return *this;
    }
    Real operator*(const Real &r) const
    {
      auto a = *this;
      a *= r;
      return a;
    }

    Real &operator/=(const Real &r)
    {
      radicand ^= r.index;
      radicand *= r.radicand.reciprocate() ^ index;
      coe /= r.coe;
      index *= r.index;
      reduce();
      return *this;
    }
    Real operator/(const Real &r) const
    {
      auto a = *this;
      a /= r;
      return a;
    }
    Real operator^(const Rational<T> &p) const
    {
      auto a = *this;
      a ^= p;
      return a;
    }
    Real &operator^=(const Rational<T> &p)
    {
      if (p.is_int())
      {
        coe ^= p;
        radicand ^= p;
      }
      else
      {
        radicand ^= p.get_numerator();
        index *= p.get_denominator();
        auto cb = coe ^ p.get_numerator();
        coe = 1;
        *this *= Real<T>{Rational<T>{1}, cb, static_cast<IndexT>(p.get_denominator())};
      }
      reduce();
      return *this;
    }
    Rational<T> &get_coe() const
    {
      return coe;
    }
    Rational<T> &get_coe()
    {
      return coe;
    }
    bool operator<(const Real &r) const
    {
      return coe * (radicand ^ r.index) < r.coe * (r.radicand * index);
    }
    bool operator==(const Real &r) const
    {
      return coe == r.coe && radicand == r.radicand && index == r.index;
    }
    bool operator!=(const Real &r) const { return !(*this == r); }
    bool operator>(const Real &r) const
    {
      return !(*this < r);
    }
    void reduce()
    {
      T num = radicand.get_numerator();
      T numbak = num;
      for (int i = 2; i < num; i++)
      {
        if (num <= 1)
          break;
        if (!is_prime(i))
          continue;
        IndexT k = 0;
        while (numbak % i == 0)
        {
          numbak /= i;
          ++k;
        }
        if (k >= index)
        {
          coe *= std::pow(i, k / index);
          num /= std::pow(i, k - k % index);
        }
        else if (numbak == 1)
        {
          auto g = gcd(index, k);
          index /= k;
          num = std::pow(i, k / g);
        }
      }

      T den = radicand.get_denominator();
      T denbak = den;
      for (int i = 2; i < den; i++)
      {
        if (den <= 1)
          break;
        if (!is_prime(i))
          continue;
        IndexT k = 0;
        while (denbak % i == 0)
        {
          denbak /= i;
          ++k;
        }
        if (k >= index)
        {
          coe /= std::pow(i, k / index);
          den /= std::pow(i, k - k % index);
        }
        else if (denbak == 1)
        {
          auto g = gcd(index, k);
          index /= k;
          den = std::pow(i, k / g);
        }
      }
      radicand = {num, den};
      if (radicand == 1)
        index = 1;
    }
    Real opposite() const { return {coe.opposite(), radicand, index}; }
    bool is_rational()
    {
      return coe == 0 || radicand == 1 || radicand == 0 || index == 1 || index == 0;
    }
    template <typename U>
    U to() const
    {
      return coe.template to<U>() *
             static_cast<U>(std::pow(radicand.template to<double>(), 1.0 / static_cast<double>(index)));
    }
    Rational<T> to() const
    {
      if (!is_rational())
        throw Error(SYMXX_ERROR_LOCATION, __func__, "Must be a rational.");
      return coe * radicand;
    }
  };
  template <typename U>
  std::ostream &
  operator<<(std::ostream &os, const Real<U> &i)
  {
    if (i.coe == 0)
    {
      os << 0;
      return os;
    }
    if (i.index == 1)
    {
      os << i.radicand * i.coe;
      return os;
    }
    if (i.radicand == 1)
    {
      os << i.coe;
      return os;
    }
    if (i.coe != 1)
      os << i.coe << "(";

    if (i.index != 2)
      os << "_" << i.index << "√" << i.radicand;
    else
      os << "√" << i.radicand;

    if (i.coe != 1)
      os << ")";
    return os;
  }

  template <typename T>
  Real<T> nth_root(std::make_unsigned_t<T> n, Rational<T> q)
  {
    return Real<T>{1, q, n};
  }
}
#endif