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
#ifndef SYMXX_FRAC_HPP
#define SYMXX_FRAC_HPP
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
  using Environment = std::shared_ptr<std::map<std::string, Real<T>>>;

  template <typename T>
  class Term
  {
    template <typename U>
    friend std::ostream &operator<<(std::ostream &os, const Term<U> &i);

  private:
    Real<T> coe;
    std::map<std::string, Rational<T>> symbols;
    Environment<T> env;

  public:
    Term(Real<T> c,
         const std::map<std::string, Rational<T>> &u = {},
         Environment<T> e = nullptr)
        : coe(c), symbols(u), env(e)
    {
      reduce();
    }
    Term(Real<T> c,
         const std::string &sym)
        : coe(c), symbols({{sym, 1}}), env(nullptr)
    {
      reduce();
    }
    Term(const Term &t)
        : coe(t.coe), symbols(t.symbols)
    {
      if (t.env != nullptr)
        env = t.env;
      reduce();
    }
    void set_env(Environment<T> e) { env = e; }
    bool operator<(const Term &t) const
    {
      auto a = get_symbols();
      auto b = t.get_symbols();
      Rational<T> aindex = 0;
      Rational<T> bindex = 0;
      for (auto &r : a)
        aindex += r.second;
      for (auto &r : b)
        bindex += r.second;

      if (aindex == bindex)
        return coe < t.coe;
      else
      {
        if (aindex == 0 || bindex != 0)
          return false;
        else if (aindex != 0 || bindex == 0)
          return true;
      }
      return aindex < bindex;
    }
    bool operator==(const Term &t) const
    {
      return coe == t.coe && symbols == t.symbols;
    }
    Term &operator*=(const Term &t)
    {
      for (auto &r : t.symbols)
      {
        auto it = symbols.find(r.first);
        if (it == symbols.end())
          symbols.insert(r);
        else
          it->second += r.second;
      }
      coe *= t.coe;
      reduce();
      return *this;
    }
    Term operator*(const Term &t) const
    {
      auto a = *this;
      a *= t;
      return a;
    }
    Term &operator/=(const Real<T> &t)
    {
      coe /= t;
      reduce();
      return *this;
    }
    Term operator/(const Real<T> &t)
    {
      auto tm = *this;
      tm /= t;
      return tm;
    }
    Term &operator^=(const Rational<T> &t)
    {
      coe ^= t;
      for (auto &r : symbols)
        r.second *= t;
      reduce();
      return *this;
    }
    Term operator^(const Rational<T> &t)
    {
      auto tm = *this;
      tm ^= t;
      return tm;
    }
    Term opposite() const { return {coe.opposite(), symbols, env}; }
    template <typename U>
    U to() const
    {
      if (!get_symbols().empty())
        throw Error(SYMXX_ERROR_LOCATION, __func__, "Term must not have symbols.");
      return get_coe().template to<U>();
    }
    Real<T> eval() const
    {
      if (!get_symbols().empty())
        throw Error(SYMXX_ERROR_LOCATION, __func__, "Term must not have symbols.");
      return get_coe();
    }
    std::unique_ptr<Real<T>> try_eval() const
    {
      if (!get_symbols().empty())
        return nullptr;
      return std::make_unique<Real<T>>(get_coe());
    }
    bool is_positive() const { return get_coe() > 0; }
    bool is_equivalent_with(const Term &t) const
    {
      return (get_symbols() == t.get_symbols()) && (get_coe().is_equivalent_with(t.get_coe()));
    }
    auto get_env() const
    {
      return env;
    }
    auto get_coe() const
    {
      if (env == nullptr)
        return coe;
      auto a = coe;
      for (auto &r : *env)
      {
        auto it = symbols.find(r.first);
        if (it == symbols.end())
          a *= r.second;
        else
          a *= r.second ^ it->second;
      }
      return a;
    }
    auto get_symbols() const
    {
      if (env == nullptr)
        return symbols;
      auto a = symbols;
      for (auto &r : *env)
      {
        auto it = a.find(r.first);
        if (it != a.end())
          a.erase(it);
      }
      return a;
    }
    void reduce()
    {
      for (auto it = symbols.begin(); it != symbols.end();)
      {
        if (it->second == 0)
          it = symbols.erase(it);
        else
          ++it;
      }
    }
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
    {
      if (coe == -1 && !symbols.empty())
        os << "-";
      else
        os << coe;
    }
    int exp = 1;
    for (auto it = symbols.begin(); it != symbols.end(); ++it)
    {
      auto exp = it->second;
      if (exp != 1)
      {
        os << "(" << it->first << "^" << exp << ")";
      }
      else
      {
        if (it->first.size() != 1)
          os << "(" << it->first << ")";
        else
          os << it->first;
      }
    }
    return os;
  }
  // return non-negative integer solutions of a1 + a2 + ... + am = n
  template <typename T>
  std::vector<std::vector<T>> solve_variable_eq_helper(T n)
  {
    std::vector<std::vector<T>> ret;
    for (T i = 0; i < n + 1; ++i)
      ret.push_back({i, n - i});
    return ret;
  }
  template <typename T>
  std::vector<std::vector<T>> solve_variable_eq(T n, T m)
  {
    if (m == 2)
      return solve_variable_eq_helper(n);
    std::vector<std::vector<T>> ret;
    auto tmp = solve_variable_eq_helper(n);
    for (auto &r : tmp)
    {
      auto t = solve_variable_eq(r[1], m - 1);
      for (auto &a : t)
      {
        ret.push_back({r[0]});
        ret.back().insert(ret.back().end(), a.begin(), a.end());
      }
    }
    return ret;
  }
  template <typename T>
  class Poly
  {
    template <typename U>
    friend std::ostream &operator<<(std::ostream &os, const Poly<U> &i);

  private:
    std::vector<Term<T>> poly;

  public:
    Poly(std::initializer_list<Term<T>> p)
        : poly(p)
    {
      reduce();
    }
    Poly &operator+=(const Poly &i)
    {
      poly.insert(poly.end(), i.poly.cbegin(), i.poly.cend());
      reduce();
      return *this;
    }
    Poly operator+(const Poly &i) const
    {
      auto a = try_eval();
      auto b = i.try_eval();
      if (a != nullptr && b != nullptr)
        return Poly{Term<T>{*a + *b}};
      auto p = *this;
      p += i;
      return p;
    }
    Poly &operator-=(const Poly &i)
    {
      *this += i.opposite();
      reduce();
      return *this;
    }
    Poly operator-(const Poly &i) const
    {
      auto a = try_eval();
      auto b = i.try_eval();
      if (a != nullptr && b != nullptr)
        return Poly{Term<T>{*a - *b}};
      auto p = *this;
      p -= i;
      return p;
    }
    Poly &operator*=(const Poly &i)
    {
      std::vector<Term<T>> tmp;
      for (auto &x : poly)
      {
        for (auto &y : i.poly)
          tmp.emplace_back(x * y);
      }
      poly = tmp;
      reduce();
      return *this;
    }
    Poly operator*(const Poly &i) const
    {
      auto a = try_eval();
      auto b = i.try_eval();
      if (a != nullptr && b != nullptr)
        return Poly{Term<T>{*a * *b}};
      Poly p = *this;
      p *= i;
      return p;
    }
    Poly &operator/=(const Real<T> &i)
    {
      for (auto &r : poly)
        r /= i;
      reduce();
      return *this;
    }
    Poly operator/(const Real<T> &i) const
    {
      auto a = try_eval();
      auto b = i.try_eval();
      if (a != nullptr && b != nullptr)
        return Poly{Term<T>{*a / *b}};
      auto p = *this;
      p /= i;
      return p;
    }
    Poly &operator^=(const Rational<T> &i)
    {
      if (i == 0)
      {
        *this = {Term<T>(1)};
        return *this;
      }
      else if (i == 1)
        return *this;
      else if (poly.size() == 1)
      {
        poly[0] ^= i;
        return *this;
      }

      std::vector<Term<T>> res;
      using pT = std::make_unsigned_t<T>;
      if (!i.is_int())
        throw Error(SYMXX_ERROR_LOCATION, __func__, "Must be a int.");
      auto avecvec = solve_variable_eq(i.to_t(), static_cast<T>(poly.size()));
      pT i_factorial = 1;
      for (pT t = 1; t <= i.to_t(); ++t)
        i_factorial *= t;
      for (auto &avec : avecvec)
      {
        Rational<T> q = i_factorial;
        for (auto &x : avec)
        {
          for (pT t = 1; t <= x; ++t)
            q /= t;
        }
        Term<T> tmp{q};
        for (pT k = 0; k < poly.size(); ++k)
          tmp *= poly[k] ^ avec[k];
        res.emplace_back(tmp);
      }
      poly = std::move(res);
      return *this;
    }

    Poly operator^(const Rational<T> &i) const
    {
      auto a = try_eval();
      if (a != nullptr)
        return Poly{Term<T>{*a ^ i}};
      auto p = *this;
      p ^= i;
      return p;
    }
    Poly opposite() const
    {
      auto a = *this;
      for (auto &r : a.poly)
        r = r.opposite();
      return a;
    }
    template <typename U>
    U to() const
    {
      U result = 0;
      for (auto &r : poly)
        result += r.template to<U>();
      return result;
    }
    Real<T> eval() const
    {
      Real<T> result = 0;
      for (auto &r : poly)
        result += r.eval();
      return result;
    }
    std::unique_ptr<Real<T>> try_eval() const
    {
      std::unique_ptr<Real<T>> result = std::make_unique<Real<T>>(0);
      for (auto &r : poly)
      {
        auto p = r.try_eval();
        if (p != nullptr)
        {
          *result += *p;
        }
        else
          return nullptr;
      }
      return result;
    }
    bool operator==(const Poly &p)
    {
      return poly == p.poly;
    }
    void set_env(Environment<T> e)
    {
      for (auto &r : poly)
        r.set_env(e);
    }
    void reduce()
    {
      std::sort(poly.begin(), poly.end());
      for (auto it = poly.begin(); it < poly.end();)
      {
        if ((it + 1) < poly.end() && it->is_equivalent_with(*(it + 1)))
        {
          *it = {it->get_coe() + (it + 1)->get_coe(), it->get_symbols(), it->get_env()};
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
      if (it->get_coe() == 0 && cnt > 0)
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
    Frac(const Real<T> &n, Environment<T> e = nullptr)
        : numerator({Term<T>{n}}), denominator({Term<T>{1}}), env(e)
    {
      reduce();
    }
    Frac(const Poly<T> &n, Environment<T> e = nullptr)
        : numerator(n), denominator({Term<T>{1}}), env(e)
    {
      reduce();
    }
    Frac(const Poly<T> &n, const Poly<T> &d, Environment<T> e = nullptr)
        : numerator(n), denominator(d), env(e)
    {
      if (denominator.is_zero())
        throw Error(SYMXX_ERROR_LOCATION, __func__, "denominator must not be 0.");
      reduce();
    }
    Frac(const Frac &t)
        : numerator(t.numerator), denominator(t.denominator)
    {
      if (t.env != nullptr)
        env = t.env;
    }
    Frac &operator+=(const Frac &t)
    {
      if (denominator == t.denominator)
      {
        numerator += t.numerator;
        return *this;
      }
      else
      {
        denominator *= t.denominator;
        numerator = numerator * t.denominator + t.numerator * denominator;
      }
      if (denominator.is_zero())
        throw Error(SYMXX_ERROR_LOCATION, __func__, "denominator must not be 0.");
      reduce();
      return *this;
    }

    Frac operator+(const Frac &t) const
    {
      auto a = try_eval();
      auto b = t.try_eval();
      if (a != nullptr && b != nullptr)
        return {*a + *b, env};
      auto c = *this;
      c += t;
      return c;
    }
    Frac &operator-=(const Frac &t)
    {
      *this += t.opposite();
      reduce();
      return *this;
    }
    Frac operator-(const Frac &t) const
    {
      auto a = try_eval();
      auto b = t.try_eval();
      if (a != nullptr && b != nullptr)
        return {*a - *b, env};
      auto c = *this;
      c -= t;
      return c;
    }
    Frac &operator*=(const Frac &t)
    {
      numerator *= t.numerator;
      denominator *= t.denominator;
      if (denominator.is_zero())
        throw Error(SYMXX_ERROR_LOCATION, __func__, "denominator must not be 0.");
      reduce();
      return *this;
    }
    Frac operator*(const Frac &t) const
    {
      auto a = try_eval();
      auto b = t.try_eval();
      if (a != nullptr && b != nullptr)
        return {*a * *b, env};
      auto c = *this;
      c *= t;
      return c;
    }
    Frac &operator/=(const Frac &t)
    {
      numerator *= t.denominator;
      denominator *= t.numerator;
      reduce();
      return *this;
    }
    Frac operator/(const Frac &t) const
    {
      auto a = try_eval();
      auto b = t.try_eval();
      if (a != nullptr && b != nullptr)
        return {*a / *b, env};
      auto c = *this;
      c /= t;
      return c;
    }
    Frac &operator^=(const Rational<T> &i)
    {
      numerator ^= i;
      denominator ^= i;
      reduce();
      return *this;
    }

    Frac operator^(const Rational<T> &i) const
    {
      auto a = try_eval();
      if (a != nullptr)
        return {*a ^ i, env};
      auto c = *this;
      c ^= i;
      return c;
    }
    void set_env(Environment<T> val)
    {
      env = val;
      numerator.set_env(val);
      denominator.set_env(val);
      reduce();
      if (denominator.is_zero())
        throw Error(SYMXX_ERROR_LOCATION, __func__, "denominator must not be 0.");
    }
    template <typename U>
    U to() const
    {
      return (numerator.template to<U>() / denominator.template to<U>());
    }
    Real<T> eval() const
    {
      return (numerator.eval() / denominator.eval());
    }
    std::unique_ptr<Real<T>> try_eval() const
    {
      auto np = numerator.try_eval();
      auto dp = denominator.try_eval();
      if (np == nullptr || dp == nullptr)
        return nullptr;
      return std::make_unique<Real<T>>(*np / *dp);
    }
    void reduce()
    {
      numerator.reduce();
      denominator.reduce();
      T den = 1;
      for (auto &r : denominator.get_poly())
        den *= r.get_coe().get_coe().get_denominator();
      for (auto &r : numerator.get_poly())
        r *= Term<T>{den, {}, env};
      for (auto &r : denominator.get_poly())
        r *= Term<T>{den, {}, env};

      T g = gcd(numerator.get_poly()[0].get_coe().get_coe().to_t(),
                denominator.get_poly()[0].get_coe().get_coe().to_t());
      for (auto &n : numerator.get_poly())
      {
        for (std::size_t i = 1; i < denominator.get_poly().size(); ++i)
        {
          T new_g = gcd(n.get_coe().get_coe().to_t(), denominator.get_poly()[i].get_coe().get_coe().to_t());
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
      return {a, denominator, env};
    }
    Frac reciprocate() const { return {denominator, numerator, env}; }
  };
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