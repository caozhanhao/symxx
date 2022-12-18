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
  using Environment = std::shared_ptr<std::map<std::string, Radical<T>>>;

  template <typename T>
  class Term
  {
    template <typename U>
    friend std::ostream &operator<<(std::ostream &os, const Term<U> &i);

  private:
    Radical<T> coe;
    std::map<std::string, Rational<T>> symbols;
    Environment<T> env;

  public:
    Term(Radical<T> c,
         const std::map<std::string, Rational<T>> &u = {},
         Environment<T> e = nullptr)
        : coe(c), symbols(u), env(e)
    {
      reduct();
    }
    Term(Radical<T> c,
         const std::string &sym)
        : coe(c), symbols({{sym, 1}}), env(nullptr)
    {
      reduct();
    }
    Term(const Term &t)
        : coe(t.coe), symbols(t.symbols)
    {
      if (env != nullptr)
        env = t.env;
      reduct();
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
      reduct();
      return *this;
    }
    Term operator*(const Term &t) const
    {
      auto a = *this;
      a *= t;
      return a;
    }
    Term &operator/=(const Radical<T> &t)
    {
      coe /= t;
      reduct();
      return *this;
    }
    Term operator/(const Radical<T> &t)
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
      reduct();
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
    Radical<T> eval() const
    {
      if (!get_symbols().empty())
        throw Error(SYMXX_ERROR_LOCATION, __func__, "Term must not have symbols.");
      return get_coe();
    }
    std::unique_ptr<Radical<T>> try_eval() const
    {
      if (!get_symbols().empty())
        return nullptr;
      return std::make_unique<Radical<T>>(get_coe());
    }
    bool is_positive() const { return get_coe() > 0; }
    bool is_equivalent_with(const Term &t) const
    {
      return (get_symbols() == t.get_symbols()) && (get_coe().is_equivalent_with(t.get_coe()));
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
    void reduct()
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
  template <typename T>
  std::vector<std::vector<T>> divide2(T n)
  {
    std::vector<std::vector<T>> ret;
    for (T i = 0; i < n + 1; ++i)
      ret.push_back({i, n - i});
    return ret;
  }
  template <typename T>
  std::vector<std::vector<T>> divide(T n, T ng)
  {
    if (ng == 2)
      return divide2(n);
    std::vector<std::vector<T>> ret;
    auto tmp = divide2(n);
    for (auto &r : tmp)
    {
      auto t = divide(r[1], ng - 1);
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
      reduct();
    }
    Poly &operator+=(const Poly &i)
    {
      poly.insert(poly.end(), i.poly.cbegin(), i.poly.cend());
      reduct();
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
      reduct();
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
      reduct();
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
    Poly &operator/=(const Radical<T> &i)
    {
      for (auto &r : poly)
        r /= i;
      reduct();
      return *this;
    }
    Poly operator/(const Radical<T> &i) const
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
      auto avecvec = divide(i.to_t(), static_cast<T>(poly.size()));
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
    Radical<T> eval() const
    {
      Radical<T> result = 0;
      for (auto &r : poly)
        result += r.eval();
      return result;
    }
    std::unique_ptr<Radical<T>> try_eval() const
    {
      std::unique_ptr<Radical<T>> result = std::make_unique<Radical<T>>(0);
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
    Frac(const Radical<T> &n)
        : numerator({Term<T>{n}}), denominator({Term<T>{1}}), env(nullptr)
    {
      reduct();
    }
    Frac(const Poly<T> &n)
        : numerator(n), denominator({Term<T>{1}}), env(nullptr)
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
    Frac(const Frac &t)
        : numerator(t.numerator), denominator(t.denominator)
    {
      if (env != nullptr)
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
      reduct();
      return *this;
    }

    Frac operator+(const Frac &t) const
    {
      auto a = try_eval();
      auto b = t.try_eval();
      if (a != nullptr && b != nullptr)
        return *a + *b;
      auto c = *this;
      c += t;
      return c;
    }
    Frac &operator-=(const Frac &t)
    {
      *this += t.opposite();
      reduct();
      return *this;
    }
    Frac operator-(const Frac &t) const
    {
      auto a = try_eval();
      auto b = t.try_eval();
      if (a != nullptr && b != nullptr)
        return *a - *b;
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
      reduct();
      return *this;
    }
    Frac operator*(const Frac &t) const
    {
      auto a = try_eval();
      auto b = t.try_eval();
      if (a != nullptr && b != nullptr)
        return *a * *b;
      auto c = *this;
      c *= t;
      return c;
    }
    Frac &operator/=(const Frac &t)
    {
      numerator *= t.denominator;
      denominator *= t.numerator;
      reduct();
      return *this;
    }
    Frac operator/(const Frac &t) const
    {
      auto a = try_eval();
      auto b = t.try_eval();
      if (a != nullptr && b != nullptr)
        return *a / *b;
      auto c = *this;
      c /= t;
      return c;
    }
    Frac &operator^=(const Rational<T> &i)
    {
      numerator ^= i;
      denominator ^= i;
      reduct();
      return *this;
    }

    Frac operator^(const Rational<T> &i) const
    {
      auto a = try_eval();
      if (a != nullptr)
        return *a ^ i;
      auto c = *this;
      c ^= i;
      return c;
    }
    Frac set_var(const std::map<std::string, Radical<T>> &val) const
    {
      Frac ret = *this;
      ret.env = std::make_shared<std::map<std::string, Radical<T>>>(val);
      ret.numerator.set_env(ret.env);
      ret.denominator.set_env(ret.env);
      ret.reduct();
      if (ret.denominator.is_zero())
        throw Error(SYMXX_ERROR_LOCATION, __func__, "denominator must not be 0.");
      return ret;
    }
    template <typename U>
    U to() const
    {
      return (numerator.template to<U>() / denominator.template to<U>());
    }
    Radical<T> eval() const
    {
      return (numerator.eval() / denominator.eval());
    }
    std::unique_ptr<Radical<T>> try_eval() const
    {
      auto np = numerator.try_eval();
      auto dp = denominator.try_eval();
      if (np == nullptr || dp == nullptr)
        return nullptr;
      return std::make_unique<Radical<T>>(*np / *dp);
    }
    void reduct()
    {
      numerator.reduct();
      denominator.reduct();
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