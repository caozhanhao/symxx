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
#ifndef SYMXX_NUM_HPP
#define SYMXX_NUM_HPP
#include "error.hpp"
#include "utils.hpp"
#include <cmath>
#include <limits>
#include <numeric>
#include <ostream>
#include <stdexcept>
#include <string>
#include <type_traits>
namespace symxx
{
  template<typename T>
  std::enable_if_t<(std::is_floating_point<T>::value), std::size_t>
  decimal_places(T v)
  {
    std::size_t count = 0;
    v = utils::Abs(v);
    auto c = v - std::floor(v);
    T factor = 10;
    T eps = std::numeric_limits<T>::epsilon() * c;
    while ((c > eps && c < (1 - eps)) && count < std::numeric_limits<T>::max_digits10)
    {
      c = v * factor;
      c = c - std::floor(c);
      factor *= 10;
      eps = std::numeric_limits<T>::epsilon() * v * factor;
      count++;
    }
    return count;
  }
  
  long double round_decplaces(long double value, int decimal_places)
  {
    const unsigned long long m = std::pow(static_cast<unsigned long long>(10), decimal_places);
    return std::round(value * m) / m;
  }
  
  template<typename T>
  class Rational
  {
    template<typename U>
    friend std::ostream &operator<<(std::ostream &os, const Rational<U> &i);

  private:
    T numerator;
    T denominator;

  public:
    template<typename U, typename R, typename = std::enable_if_t<
        std::is_arithmetic_v<std::decay_t<U>>
        || std::is_same_v<std::decay_t<U>, T>>,
        typename = std::enable_if_t<
            std::is_arithmetic_v<std::decay_t<R>>
            || std::is_same_v<std::decay_t<R>, T>>>
    Rational(U n, R d)
        : numerator(n), denominator(d)
    {
      if (denominator == 0)
      {
        throw Error("denominator must not be 0.");
      }
      reduce();
    }
  
    template<typename U, typename = std::enable_if_t<
        std::is_arithmetic_v<std::decay_t<U>>
        || std::is_same_v<std::decay_t<U>, T>>>
    Rational(U n)
        : numerator(n), denominator(1)
    {
      if (denominator == 0) throw Error("denominator must not be 0.");
      reduce();
    }
  
    Rational()
        : numerator(0), denominator(1)
    {
    }
  
    Rational(long double x)
    {
      auto ndigits = decimal_places(x);
      long double fracpart, intpart;
      fracpart = std::modf(x, &intpart);
      if constexpr (std::is_fundamental_v<T>)
      {
        if (ndigits > std::numeric_limits<T>::digits10)
        {
          x = round_decplaces(x, std::numeric_limits<T>::digits10);
        }
        denominator = std::pow(10,
                               ndigits > std::numeric_limits<T>::digits10
                               ? std::numeric_limits<T>::digits10
                               : ndigits);
        numerator = x * denominator;
      }
      else
      {
        denominator = static_cast<T>(std::pow(10, ndigits));
        numerator = denominator * x;
      }
      reduce();
    }
  
    Rational(const std::string &n)
    {
      try
      {
        if (n.find('.') != std::string::npos)
        {
          *this = Rational(std::stold(n));
        }
        else
        {
          auto k = n.find('/');
          if (k == std::string::npos)
          {
            numerator = utils::To_int<T>(n);
            denominator = 1;
          }
          else
          {
            numerator = utils::To_int<T>(n.substr(0, k));
            denominator = utils::To_int<T>(n.substr(k + 1));
          }
        }
      }
      catch (std::invalid_argument &)
      {
        throw Error("Invaild string.");
      }
      catch (std::out_of_range &)
      {
        throw Error("The number is out of range.");
      }
      if (denominator == 0)
        throw Error("denominator must not be 0.");
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
        throw Error("denominator must not be 0.");
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
      *this += i.negate();
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
        throw Error("denominator must not be 0.");
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
  
    Rational pow(const Rational<T> &p) const
    {
      if (p == 0) return 1;
      if (p == 1) return *this;
      Rational<T> res;
      res.numerator = static_cast<T>(utils::Pow(numerator, p.template to<double>()));
      res.denominator = static_cast<T>(utils::Pow(denominator, p.template to<double>()));
      if (denominator == 0)
      {
        throw Error("denominator must not be 0.");
      }
      res.reduce();
      return res;
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
  
    Rational negate() const
    {
      return Rational{-numerator, denominator};
    }
  
    Rational reciprocate() const { return {denominator, numerator}; }
  
    void reduce()
    {
      T g = ::symxx::utils::Gcd(::symxx::utils::Abs(numerator), ::symxx::utils::Abs(denominator));
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
  
    template<typename U>
    U to() const
    {
      return static_cast<U>(numerator) / static_cast<U>(denominator);
    }
  
    T to_t() const
    {
      return numerator / denominator;
    }
  };
  
  template<typename U>
  std::ostream &
  operator<<(std::ostream &os, const Rational<U> &i)
  {
    if (i.denominator != 1)
    {
      os << "(" << i.numerator << "/" << i.denominator << ")";
    }
    else
    {
      os << i.numerator;
    }
    return os;
  }
  
  template<typename T>
  bool is_prime(T a)
  {
    T i = 2;
    while (i < a)
    {
      if (a % i == 0)
      {
        break;
      }
      i++;
    }
    if (i == a)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  
  template<typename T>
  void radical_reduce(T &num, utils::Make_unsigned_t<T> &index, Rational<T> &coe, bool is_numerator)
  {
    auto numbak = num;
    for (T i = 2; i < num; i++)
    {
      if (num <= 1)
      {
        break;
      }
      if (!is_prime(i))
      {
        continue;
      }
      utils::Make_unsigned_t<T> k = 0;
      while (numbak % i == 0)
      {
        numbak /= i;
        ++k;
      }
      if (k == 0)
        continue;
      if (k >= index)
      {
        if (is_numerator)
        {
          coe *= static_cast<T>(utils::Pow(i, k / index));
        }
        else
        {
          coe /= static_cast<T>(utils::Pow(i, k / index));
        }
        num /= utils::Pow(i, k - k % index);
      }
      else if (numbak == 1)
      {
        auto g = utils::Gcd(index, k);
        index /= k;
        num = utils::Pow(i, k / g);
      }
    }
  }
  
  namespace num_internal
  {
    struct NormalTag {};
    struct RationalTag {};
    template<typename T, typename U>
    struct TagDispatch
    {
      using tag = std::conditional_t<std::is_same_v<U, Rational<T>>, RationalTag, NormalTag>;
    };
  }
  template<typename T>
  class Real
  {
    template<typename U>
    friend std::ostream &operator<<(std::ostream &os, const Real<U> &i);
  
  private:
    using IndexT = utils::Make_unsigned_t<T>;
    IndexT index;
    Rational<T> radicand;
    Rational<T> coe;
  
  public:
    Real(Rational<T> c, Rational<T> r = 1, IndexT i = 1)
        : index(i), radicand(std::move(r)), coe(std::move(c)) { reduce(); }
    
    template<typename U, typename = std::enable_if_t<
        std::is_arithmetic_v<std::decay_t<U>>
        || std::is_same_v<std::decay_t<U>, T>>>
    Real(U c)
        : index(1), radicand(1), coe(c) { reduce(); }
    
    Real() : index(1), radicand(1), coe(0) {}
    
    bool is_equivalent_with(const Real &t) const
    {
      if (radicand == t.radicand && radicand == 1)
      {
        return true;
      }
      if (index == t.index && index == 1)
        return true;
      return (radicand == t.radicand && index == t.index);
    }
    
    Real &operator+=(const Real &r)
    {
      if (!is_equivalent_with(r))
        throw Error("radicand and index must be the same.");
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
      *this += r.negate();
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
      radicand = radicand.pow(r.index);
      radicand *= r.radicand.pow(index);
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
      radicand = radicand.pow(r.index);
      radicand *= r.radicand.reciprocate().pow(index);
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
    
    Real pow(const Rational<T> &p) const
    {
      if (p == 0) return 1;
      if (p == 1) return *this;
      if (p < 0)
      {
        return pow(p.negate());
      }
      
      auto res = *this;
      if (p.is_int())
      {
        res.coe = res.coe.pow(p);
        res.radicand = res.radicand.pow(p);
      }
      else
      {
        res.radicand = res.radicand.pow(p.get_numerator());
        res.index *= p.get_denominator();
        auto cb = res.coe.pow(p.get_numerator());
        res.coe = 1;
        res *= Real<T>{Rational<T>{1}, cb, static_cast<IndexT>(p.get_denominator())};
      }
      res.reduce();
      return res;
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
      return coe * (radicand.pow(r.index)) < r.coe * (r.radicand * index);
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
      radical_reduce(num, index, coe, true);
      T den = radicand.get_denominator();
      radical_reduce(den, index, coe, false);
      radicand = {num, den};
      if (radicand == 1)
      {
        index = 1;
      }
    }
    
    Real negate() const { return {coe.negate(), radicand, index}; }
    
    bool is_rational() const
    {
      return coe == 0 || radicand == 1 || radicand == 0 || index == 1 || index == 0;
    }
    
    Real reciprocate() const
    {
      auto p = *this;
      p.coe = p.coe.reciprocate();
      p.radicand = p.radicand.reciprocate();
      return p;
    }
    
    template<typename U>
    U to() const
    {
      return internal_to<U>(typename num_internal::TagDispatch<T, U>::tag{});
    }
  
  private:
    template<typename U>
    U internal_to(num_internal::NormalTag) const
    {
      return coe.template to<U>() *
             static_cast<U>(utils::Pow(radicand.template to<double>(), 1.0 / static_cast<double>(index)));
    }
    
    template<typename U>
    U internal_to(num_internal::RationalTag) const
    {
      if (!is_rational())
      {
        throw Error("Must be a rational.");
      }
      return coe * radicand;
    }
  };
  
  template<typename U>
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
    {
      os << "_" << i.index << "√" << i.radicand;
    }
    else
    {
      os << "√" << i.radicand;
    }
  
    if (i.coe != 1)
    {
      os << ")";
    }
    return os;
  }
  
  template<typename T>
  Real<T> nth_root(utils::Make_unsigned_t<T> n, Rational<T> q)
  {
    return Real<T>{1, q, n};
  }
}
#endif