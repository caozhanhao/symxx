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
#include "int_adapter.hpp"
#include <cmath>
#include <limits>
#include <numeric>
#include <ostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <memory>
namespace symxx
{
  template<typename T>
  std::enable_if_t<(std::is_floating_point<T>::value), std::size_t>
  decimal_places(T v)
  {
    std::size_t count = 0;
    v = Abs(v);
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
      symxx_assert(denominator != 0, symxx_division_by_zero);
      normalize();
    }
  
    template<typename U, typename = std::enable_if_t<
        std::is_arithmetic_v<std::decay_t<U>>
        || std::is_same_v<std::decay_t<U>, T>>>
    Rational(U n)
        : numerator(n), denominator(1)
    {
      symxx_assert(denominator != 0, symxx_division_by_zero);
      normalize();
    }
  
    Rational()
        : numerator(0), denominator(1)
    {
    }
  
    Rational(double x) : Rational(static_cast<long double>(x))
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
        auto tmp = Pow<unsigned long long>(10, ndigits);
        denominator = static_cast<T>(tmp);
        numerator = x * tmp;
      }
      normalize();
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
            numerator = To_int<T>(n);
            denominator = 1;
          }
          else
          {
            numerator = To_int<T>(n.substr(0, k));
            denominator = To_int<T>(n.substr(k + 1));
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
      symxx_assert(denominator != 0, symxx_division_by_zero);
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
      normalize();
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
      symxx_assert(denominator != 0, symxx_division_by_zero);
      normalize();
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
      *this *= i.inverse();
      normalize();
      return *this;
    }
  
    Rational pow(const Rational<T> &p) const
    {
      if (p == 0) return 1;
      if (p == 1) return *this;
      Rational<T> res;
      res.numerator = static_cast<T>(Pow(numerator, p.template to<double>()));
      res.denominator = static_cast<T>(Pow(denominator, p.template to<double>()));
      symxx_assert(denominator != 0, symxx_division_by_zero);
      res.normalize();
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
  
    bool is_int() const { return denominator == 1; }
  
    Rational negate() const
    {
      return Rational{-numerator, denominator};
    }
  
    Rational inverse() const { return {denominator, numerator}; }
  
    void normalize()
    {
      T g = ::symxx::Gcd(::symxx::Abs(numerator), ::symxx::Abs(denominator));
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
  
    template<typename U>
    std::unique_ptr<U> try_to() const
    {
      try
      {
        return std::make_unique<U>(static_cast<U>(numerator) / static_cast<U>(denominator));
      }
      catch (...)
      {
        return nullptr;
      }
      symxx_unreachable();
      return nullptr;
    }
  
    T to_t() const
    {
      return numerator / denominator;
    }
  
    std::string to_string() const
    {
      if (denominator != 1)
      {
        return To_String(numerator) + "/" + To_String(denominator);
      }
      else
        return To_String(numerator);
      symxx_unreachable();
      return "";
    }
  };
  
  template<typename U>
  std::ostream &
  operator<<(std::ostream &os, const Rational<U> &i)
  {
    os << i.to_string();
    return os;
  }
  
  template<typename T>
  void radical_reduce(T &num, Make_unsigned_t<T> &index, Rational<T> &coe)
  {
    //unfinished
    for (T i = 2; i < num && num > 1; i++)
    {
      Make_unsigned_t<T> k = 0;
      auto n = num;
      for (; n % i == 0; n /= i) ++k;
      if (k == 0 || k == 1) continue;
      if (k >= index)
      {
        coe *= static_cast<T>(Pow(i, k / index));
        num /= Pow(i, k - k % index);
      }
      else if (n == 1)
      {
        auto g = Gcd(index, k);
        index /= g;
        num = Pow(i, k / g);
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
  private:
    using IndexT = Make_unsigned_t<T>;
    IndexT index;
    Rational<T> radicand;
    Rational<T> coe;
  
  public:
    Real(Rational<T> c, Rational<T> r = 1, IndexT i = 1)
        : index(i), radicand(std::move(r)), coe(std::move(c)) { normalize(); }
  
    template<typename U, typename = std::enable_if_t<
        std::is_arithmetic_v<std::decay_t<U>>
        || std::is_same_v<std::decay_t<U>, T>>>
    Real(U c)
        : index(1), radicand(1), coe(c) { normalize(); }
    
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
      normalize();
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
      normalize();
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
      radicand = radicand.pow(r.index) * r.radicand.pow(index);
      index *= r.index;
      coe *= r.coe;
      normalize();
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
      *this *= r.inverse();
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
      if (p < 0) return pow(p.negate());
      
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
      res.normalize();
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
      return coe.pow(r.index * index) * radicand.pow(r.index) < r.coe.pow(r.index * index) * (r.radicand.pow(index));
    }
    
    bool operator==(const Real &r) const
    {
      return coe == r.coe && radicand == r.radicand && index == r.index;
    }
    
    bool operator!=(const Real &r) const { return !(*this == r); }
    
    bool operator>(const Real &r) const
    {
      return !(*this < r || *this == r);
    }
  
    void normalize()
    {
      if (!radicand.is_int())
      {
        coe /= radicand.get_denominator();
        radicand *= Pow(radicand.get_denominator(), index);
      }
      T num = radicand.get_numerator();
      radical_reduce(num, index, coe);
      radicand = {num, 1};
      if (radicand == 1) index = 1;
    }
  
    Real negate() const { return {coe.negate(), radicand, index}; }
    
    bool is_rational() const
    {
      return coe == 0 || radicand == 1 || radicand == 0 || index == 1 || index == 0;
    }
  
    Real inverse() const
    {
      auto p = *this;
      p.coe = p.coe.inverse();
      p.radicand = p.radicand.inverse();
      return p;
    }
  
    std::string to_string() const
    {
      if (coe == 0) return "0";
      if (is_rational()) return coe.to_string();
  
      std::string ret;
      if (coe != 1)
      {
        ret += coe.to_string();
      }
      if (index != 2)
      {
        ret += "_" + To_String(index) + "_/" + radicand.to_string();
      }
      else
      {
        ret += "_/" + radicand.to_string();
      }
      return ret;
    }
  
    template<typename U>
    U to() const
    {
      return internal_to<U>(typename num_internal::TagDispatch<T, U>::tag{});
    }
  
    template<typename U>
    std::unique_ptr<U> try_to() const
    {
      return internal_try_to<U>(typename num_internal::TagDispatch<T, U>::tag{});
    }

  private:
    template<typename U>
    U internal_to(num_internal::NormalTag) const
    {
      return coe.template to<U>() *
             static_cast<U>(Pow(radicand.template to<double>(), 1.0 / static_cast<double>(index)));
    }
  
    template<typename U>
    U internal_to(num_internal::RationalTag) const
    {
      symxx_assert(is_rational(), "Must be a rational.");
      return coe * radicand;
    }
  
    template<typename U>
    std::unique_ptr<U> internal_try_to(num_internal::NormalTag) const
    {
      try
      {
        return std::make_unique<U>(coe.template to<U>() *
                                   static_cast<U>(Pow(radicand.template to<double>(),
                                                      1.0 / static_cast<double>(index))));
      }
      catch (...)
      {
        return nullptr;
      }
      symxx_unreachable();
      return nullptr;
    }
  
    template<typename U>
    U internal_try_to(num_internal::RationalTag) const
    {
      symxx_assert(is_rational(), "Must be a rational.");
      return coe * radicand;
    }
  };
  
  template<typename U>
  std::ostream &
  operator<<(std::ostream &os, const Real<U> &i)
  {
    os << i.to_string();
    return os;
  }
  
  template<typename T>
  Real<T> nth_root(const Make_unsigned_t<T> &n, const Rational<T> &q)
  {
    return Real<T>{1, q, n};
  }
}
#endif