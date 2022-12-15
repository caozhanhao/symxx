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
#ifndef SYMXX_EXPR_HPP
#define SYMXX_EXPR_HPP
#include "num.hpp"
#include "error.hpp"
#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace symxx
{

  template <typename T>
  using Environment = std::shared_ptr<std::map<std::string, Rational<T>>>;

  template <typename T>
  class Term
  {
    template <typename U>
    friend std::ostream &operator<<(std::ostream &os, const Term<U> &i);

  private:
    Rational<T> coe;
    std::vector<std::string> symbols;
    Environment<T> env;

  public:
    Term(Rational<T> c,
         std::vector<std::string> u = {},
         Environment<T> e = nullptr)
        : coe(c), symbols(u), env(e)
    {
    }
    void set_env(Environment<T> e) { env = e; }
    bool operator<(const Term &t) const
    {
      auto a = get_symbols();
      auto b = t.get_symbols();
      if (a == b)
        return coe < t.coe;
      return a < b;
    }
    bool operator==(const Term &t) const
    {
      return coe == t.coe && symbols == t.symbols;
    }
    Term &operator*=(const Term &t)
    {
      symbols.insert(symbols.cend(), t.symbols.cbegin(), t.symbols.cend());
      coe *= t.coe;
      return *this;
    }
    Term operator*(const Term &t) const
    {
      auto a = *this;
      a *= t;
      return a;
    }
    Term &operator/=(const Rational<T> &t)
    {
      coe /= t;
      return *this;
    }
    Term operator/(const Rational<T> &t)
    {
      auto tm = *this;
      tm /= t;
      return tm;
    }
    Term opposite() const { return {coe.opposite(), symbols}; }
    bool is_positive() const { return get_coe() > 0; }
    bool is_equivalent_with(const Term &t) const
    {
      return get_symbols() == t.get_symbols();
    }
    auto get_coe() const
    {
      if (env == nullptr)
        return coe;
      auto a = coe;
      for (auto &r : *env)
        a *= (r.second ^ std::count(symbols.cbegin(), symbols.cend(), r.first));
      return a;
    }
    auto get_symbols() const
    {
      if (env == nullptr)
        return symbols;
      auto a = symbols;
      a.erase(
          std::remove_if(a.begin(),
                         a.end(),
                         [this](auto &&i)
                         { return env->find(i) != env->end(); }),
          a.end());
      return a;
    }
  };

  template <typename T>
  class Poly
  {
    template <typename U>
    friend std::ostream &operator<<(std::ostream &os, const Poly<U> &i);

  private:
    std::vector<Term<T>> poly;

  public:
    Poly(std::vector<Term<T>> p)
        : poly(p)
    {
      reduct();
    }
    Poly &operator+(const Poly &i)
    {
      poly.insert(poly.end(), i.cbegin(), i.cend());
      return *this;
    }
    Poly operator+(const Poly &i) const
    {
      auto a = *this;
      a += i;
      return a;
    }
    Poly &operator-=(const Poly &i)
    {
      *this += i.opposite();
      return *this;
    }
    Poly operator-(const Poly &i) const
    {
      auto p = *this;
      p -= i;
      return p;
    }
    Poly &operator*=(const Poly &i)
    {
      for (auto &x : poly)
      {
        for (auto &y : i.poly)
          poly.emplace_back(x * y);
      }
      return *this;
    }
    Poly operator*(const Poly &i) const
    {
      Poly a = *this;
      a *= i;
      return a;
    }
    Poly &operator/=(const Rational<T> &i)
    {
      for (auto &r : poly)
        r /= i;
      return *this;
    }
    Poly operator/(const Rational<T> &i) const
    {
      auto p = *this;
      p /= i;
      return p;
    }
    Poly opposite() const
    {
      auto a = poly;
      for (auto &r : a)
        r = r.opposite();
      return {a};
    }
    void set_env(Environment<T> e)
    {
      for (auto &r : poly)
        r.set_env(e);
    }
    void reduct()
    {
      std::sort(poly.begin(), poly.end());
      for (auto it = poly.begin(); it < poly.end();)
      {
        if ((it + 1) < poly.end() && it->is_equivalent_with(*(it + 1)))
        {
          *it = {it->get_coe() + (it + 1)->get_coe(), it->get_symbols()};
          it = poly.erase(it + 1);
          --it;
        }
        else
          ++it;
      }
    }
    bool is_zero() const
    {
      for (auto &r : poly)
      {
        if (r.get_coe() != 0)
          return false;
      }
      return true;
    }
    auto &get_poly() const { return poly; }
    auto &get_poly() { return poly; }
  };
  template <typename T>
  class Frac
  {
    template <typename U>
    friend std::ostream &operator<<(std::ostream &os, const Frac<U> &i);

  private:
    Poly<T> numerator;
    Poly<T> denominator;
    Environment<T> env;

  public:
    Frac(const Poly<T> &n)
        : numerator(n), denominator({{1}}), env(nullptr)
    {
      reduct();
    }
    Frac(const Poly<T> &n, const Poly<T> &d)
        : numerator(n), denominator(d), env(nullptr)
    {
      if (denominator.is_zero())
        throw Error(SYMXX_ERROR_LOCATION, __func__, "denominator must not be 0.");
      reduct();
    }

    Frac &operator+=(const Frac &t)
    {
      if (denominator == t.denominator)
      {
        numerator.insert(numerator.cend(), t.numerator.cbegin(), t.numerator.cend());
        return *this;
      }
      else
      {
        denominator *= t.denominator;
        numerator = numerator * t.denominator + t.numerator * denominator;
      }
      if (denominator.is_zero())
        throw Error(SYMXX_ERROR_LOCATION, __func__, "denominator must not be 0.");
      return *this;
    }

    Frac operator+(const Frac &t) const
    {
      auto a = *this;
      a += t;
      return a;
    }
    Frac &operator-=(const Frac &t)
    {
      *this += t.opposite();
      return *this;
    }
    Frac operator-(const Frac &t) const
    {
      auto a = *this;
      a -= t;
      return a;
    }
    Frac &operator*=(const Frac &t)
    {
      numerator *= t.numerator;
      denominator *= t.denominator;
      if (denominator.is_zero())
        throw Error(SYMXX_ERROR_LOCATION, __func__, "denominator must not be 0.");
      return *this;
    }
    Frac operator*(const Frac &t) const
    {
      auto a = *this;
      a *= t;
      return a;
    }
    Frac &operator/=(const Frac &t)
    {
      numerator *= t.denominator;
      denominator *= t.numerator;
      return *this;
    }
    Frac operator/(const Frac &t) const
    {
      auto a = *this;
      a /= t;
      return a;
    }
    Frac set_var(const std::map<std::string, Rational<T>> &val) const
    {
      Frac ret = *this;
      ret.env = std::make_shared<std::map<std::string, Rational<T>>>(val);
      ret.numerator.set_env(ret.env);
      ret.denominator.set_env(ret.env);
      ret.reduct();
      if (ret.denominator.is_zero())
        throw Error(SYMXX_ERROR_LOCATION, __func__, "denominator must not be 0.");
      return ret;
    }

    void reduct()
    {
      numerator.reduct();
      denominator.reduct();
      T den = 1;
      for (auto &r : denominator.get_poly())
        den *= r.get_coe().get_denominator();
      for (auto &r : numerator.get_poly())
        r *= Term<T>{den, {}, env};
      for (auto &r : denominator.get_poly())
        r *= Term<T>{den, {}, env};

      T g = gcd(numerator.get_poly()[0].get_coe().to_t(), denominator.get_poly()[0].get_coe().to_t());
      for (auto &n : numerator.get_poly())
      {
        for (std::size_t i = 1; i < denominator.get_poly().size(); ++i)
        {
          T new_g = gcd(n.get_coe().to_t(), denominator.get_poly()[i].get_coe().to_t());
          if (g % new_g == 0)
            g = std::min(g, new_g);
          else
            return;
        }
      }
      numerator /= g;
      denominator /= g;
    }
    Frac opposite() const
    {
      auto a = numerator.opposite();
      return {a, denominator};
    }
    Frac reciprocate() const { return {denominator, numerator}; }
  };
  template <typename U>
  std::ostream &
  operator<<(std::ostream &os, const Term<U> &i)
  {
    auto coe = i.get_coe();
    auto symbols = i.get_symbols();
    if (coe == 0)
    {
      os << 0;
      return os;
    }
    if (coe != 1 || symbols.empty())
      os << coe;
    int exp = 1;
    for (auto it = symbols.begin(); it != symbols.end(); ++it)
    {
      if (std::next(it) != symbols.end() && *it == *std::next(it))
      {
        exp++;
        continue;
      }
      if (exp != 1)
      {
        os << "(" << *it << "^" << exp << ")";
        exp = 1;
      }
      else
      {
        if (it->size() != 1)
          os << "(" << *it << ")";
        else
          os << *it;
      }
    }
    return os;
  }
  template <typename U>
  std::ostream &
  operator<<(std::ostream &os, const Poly<U> &i)
  {
    if (i.poly.empty())
      return os;

    auto cnt = std::count_if(i.poly.cbegin(), i.poly.cend(), [](auto &&f)
                             { return f.get_coe() != 0; });
    if (cnt > 1)
      os << "(";
    bool first = true;
    for (auto it = i.poly.begin(); it < i.poly.end(); ++it)
    {
      if (it->get_coe() == 0)
        continue;
      if (first)
      {
        os << *it;
        first = false;
        continue;
      }
      if (!it->is_positive())
      {
        os << "-";
        os << it->opposite();
        continue;
      }
      else
      {
        os << "+";
        os << *it;
        continue;
      }
    }
    if (cnt > 1)
      os << ")";
    return os;
  }
  template <typename U>
  std::ostream &
  operator<<(std::ostream &os, const Frac<U> &i)
  {
    if (i.numerator.get_poly().empty())
    {
      os << "0";
      return os;
    }

    if (i.denominator.get_poly().size() == 1 && i.denominator.get_poly()[0].get_coe() == 1 &&
        i.denominator.get_poly()[0].get_symbols().empty())
    {
      os << i.numerator;
      return os;
    }

    os << "(" << i.numerator << "/" << i.denominator << ")";
    return os;
  }
}
#endif