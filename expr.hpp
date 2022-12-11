#include "num.hpp"
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <memory>
namespace expr {

template <typename T>
using Environment = std::shared_ptr<std::map<std::string, num::Rational<T>>>;

template <typename T>
class Term {
  template <typename U>
  friend std::ostream &operator<<(std::ostream &os, const Term<U> &i);

private:
  num::Rational<T> coe;
  std::vector<std::string> symbols;
  Environment<T> env;

public:
  Term(num::Rational<T> c, std::vector<std::string> u = {}, Environment<T> e = nullptr)
      : coe(c), symbols(u), env(e)
        {}
  void set_env(Environment<T> e){env = e;}
  bool operator<(const Term &t) const {
    if (symbols == t.symbols)
      return coe < t.coe;
    return symbols < t.symbols;
  }
  bool operator==(const Term &t) const {
    return coe == t.coe && symbols == t.symbols;
  }
  Term operator*(const Term &t) const {
    auto a = symbols;
    a.insert(a.cend(), t.symbols.cbegin(), t.symbols.cend());
    return {coe * t.coe, a, env};
  }
  Term opposite() const { return {coe.opposite(), symbols}; }
  bool is_positive() const {
    return get_coe() > 0;
    }
  bool is_equivalent_with(const Term& t) const {return get_symbols() == t.get_symbols();}
  auto get_coe() const {
    if(env == nullptr) return coe;
    auto a = coe;
    for (auto &r : *env) 
      a *= (r.second ^ std::count(symbols.cbegin(), symbols.cend(), r.first));
    return a;
  }
  auto get_symbols() const {
    if (env == nullptr)
      return symbols;
    auto a = symbols;
    a.erase(std::remove_if(a.begin(), a.end(),
                           [this](auto&& i){return env->find(i) != env->end();}), a.end());
    return a;
  }
};
template <typename T>
void single_reduct(std::vector<Term<T>> &t) {
  std::sort(t.begin(), t.end());
  for (auto it = t.begin(); it < t.end();) {
    if ((it + 1) < t.end() && it->is_equivalent_with(*(it + 1))) {
      *it = {it->get_coe() + (it + 1)->get_coe(), it->get_symbols()};
      it = t.erase(it + 1);
    } else
      ++it;
  }
}
template <typename T>
class Frac {
    template <typename U>
    friend std::ostream &operator<<(std::ostream &os, const Frac<U> &i);

  private:
    std::vector<Term<T>> numerator;
    std::vector<Term<T>> denominator;
    Environment<T> env;
  public:
    Frac(const std::vector<Term<T>>& n) :numerator(n), denominator({{1, {}}}), env(nullptr) { reduct(); }
    Frac(const std::vector<Term<T>> &n, const std::vector<Term<T>> &d)
        : numerator(n), denominator(d), env(nullptr) {
    reduct();
    }

    Frac operator+(const Frac &t) const
    {
    if (denominator == t.denominator) {
      auto a = numerator;
      a.insert(a.cend(), t.numerator.cbegin(), t.numerator.cend());
      return {a};
    } else {
      std::vector<Term<T>> den;
      for (auto &x : denominator) {
        for (auto &y : t.denominator)
          den.emplace_back(x * y);
      }

      std::vector<Term<T>> num;
      for (auto &x : numerator) {
        for (auto &y : t.denominator)
          num.emplace_back(x * y);
      }
      for (auto &x : t.numerator) {
        for (auto &y : denominator)
          num.emplace_back(x * y);
      }
      return {num, den};
      }
    }


    Frac operator-(const Frac &t) const {
    return *this + t.opposite();
    }
    Frac operator*(const Frac &t) const {
    std::vector<Term<T>> num;
    for (auto &x : numerator) {
      for (auto &y : t.numerator)
       num.emplace_back(x * y);
    }

    std::vector<Term<T>> den;
    for (auto &x : denominator) {
      for (auto &y : t.denominator)
       den.emplace_back(x * y);
    }
    return {num, den};
    }
    Frac operator/(const Frac &t) const {
    return *this * t.reciprocate();
    }
    Frac set_var(const std::map<std::string, num::Rational<T>> &val) const {
      Frac ret = *this;
      ret.env = std::make_shared<std::map<std::string, num::Rational<T>>>(val);
      for (auto &r : ret.numerator)
       r.set_env(ret.env);
      for (auto &r : ret.denominator)
       r.set_env(ret.env);
    ret.reduct();
    return ret;
    }

  private:
    void reduct() {
    single_reduct(numerator);
    single_reduct(denominator);
    T den = 1;
    for (auto &r : denominator)
      den *= r.get_coe().get_denominator();
    for (auto &r : numerator)
      r = r * Term<T>{den, {}, env};
    for (auto &r : denominator)
      r = r * Term<T>{den, {}, env};

    if (numerator.size() != denominator.size())
      return;

    num::Rational<T> g = 0;
    for (std::size_t i = 0; i < numerator.size(); ++i) {
      if (!numerator[i].is_equivalent_with(denominator[i]))
       return;
      if(g == 0)
       g = numerator[i].get_coe() / denominator[i].get_coe();
      else if (g != numerator[i].get_coe() / denominator[i].get_coe()) return;
    }
    numerator = {{g, {}, env}};
    denominator = {{1, {}, env}};
    }
  Frac opposite() const{
    auto a = numerator;
    for (auto &r : a)
      r = r.opposite();
    return {a};
}
Frac reciprocate() const {
    return {denominator, numerator};
}
};
template <typename U>
std::ostream &operator<<(std::ostream &os, const Term<U> &i) {
  auto coe = i.get_coe();
  auto symbols = i.get_symbols();
  if (coe == 0) {
    os << 0;
    return os;
  }
if (coe != 1 || symbols.empty())
    os << coe;
int exp = 1;
for (auto it = symbols.begin(); it != symbols.end(); ++it) {
    if (std::next(it) != symbols.end() && *it == *std::next(it)) {
      exp++;
      continue;
    }
      if (exp != 1) {
      os << "(" << *it << "^" << exp << ")";
      exp = 1;
      } else {
      if (it->size() != 1)
        os << "(" << *it <<  ")";
      else
        os << *it;
      }
}
      return os;
}
template <typename U>
std::ostream &single_output(std::ostream &os, const std::vector<Term<U>> &i) {
      if (i.empty())
        return os;
      if(i.size() > 1)
        os << "(";
      os << i[0];
      for (auto it = i.begin() + 1; it < i.end(); ++it) {
        if (!it->is_positive()) {
          os << "-";
          os << it->opposite();
        }
        else {
          os << "+";
          os << *it;
        }
      }
      if(i.size() > 1)
        os << ")";
      return os;
}
template <typename U>
std::ostream &operator<<(std::ostream &os, const Frac<U> &i) {
      if (i.numerator.empty()) {
        os << "0";
        return os;
      }

      if (i.denominator.size() == 1 && i.denominator[0].get_coe() == 1 &&
          i.denominator[0].get_symbols().empty()) {
        single_output(os, i.numerator);
        return os;
      }

      os << "(";
      single_output(os, i.numerator);
      os << "/";
      single_output(os, i.denominator);
      os << ")";
      return os;
}
} // namespace expr