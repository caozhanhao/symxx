#include <limits>
#include <ostream>
#include <string>
#include <type_traits>
#include <numeric>
namespace num{
inline int gcd(int a, int b) { return b > 0 ? gcd(b, a % b) : a; }
template <typename T,
          typename = typename std::enable_if_t<std::is_integral<T>::value>>
class Rational {
  template <typename U>
  friend std::ostream &operator<<(std::ostream &os, const Rational<U> &i);

private:
  T numerator;
  T denominator;

public:
  Rational(T n) : numerator(n), denominator(1) {}
  Rational(T n, T d) : numerator(n), denominator(d) { reduct(); }
  Rational() : numerator(0), denominator(1) {}
  Rational(const std::string& n) {
    auto k = n.find('/');
    if (k == std::string::npos) {
      numerator = std::stoll(n);
      denominator = 1;
    }
    else {
      numerator = std::stoll(n.substr(0, k));
      denominator = std::stoll(n.substr(k + 1));
    }
  }
  Rational operator+(const Rational &i) const {
    return {numerator * i.denominator + i.numerator * denominator,
            denominator * i.denominator};
  }
  Rational &operator+=(const Rational &i) {
    *this = *this + i;
    return *this;
}
Rational operator-(const Rational &i) const { return *this + i.opposite(); }
Rational &operator-=(const Rational &i) {
    *this = *this - i;
    return *this;
  }
  Rational operator*(const Rational &i) const {
    return { numerator * i.numerator ,denominator * i.denominator};
  }
  Rational &operator*=(const Rational &i) {
    *this = *this * i;
    return *this;
  }
  Rational operator/(const Rational &i) const {
    return *this * i.reciprocate();
  }
  Rational &operator/=(const Rational &i) {
    *this = *this / i.reciprocate();
    return *this;
  }
  Rational operator^(std::make_unsigned_t<T> p) const {
     return {static_cast<T>(std::pow(numerator, p)), static_cast<T>
     (std::pow(denominator, p))};
  }
  Rational &operator^=(std::make_unsigned_t<T> p) {
    *this = *this ^ p;
    return *this;
  }
  bool operator<(const Rational &r) const {
    return numerator * r.denominator < r.numerator * denominator;
  }
  bool operator==(const Rational &r) const {
    return numerator == r.numerator && denominator == r.denominator;
  }
  bool operator!=(const Rational &r) const {
    return !(*this == r);
  }
  bool operator>(const Rational &r) const {
    return numerator * r.denominator > r.numerator * denominator;
  }
  Rational opposite() const {
    return {-numerator, denominator};
  }
  Rational reciprocate()  const{
    return {denominator, numerator};
  }
  void reduct() {
    T g = gcd(std::abs(numerator), std::abs(denominator));
    numerator /= g;
    denominator /= g;
    if (denominator == -1) {
      numerator = -numerator;
      denominator = 1;
      }
  }
  T get_numerator() const { return numerator; }
  T get_denominator() const { return denominator; }
  template<typename U>
  U to() const {
    return static_cast<U>(numerator) / static_cast<U>(denominator);
  }
};
template<typename U>
std::ostream &operator<<(std::ostream &os, const Rational<U> &i) {
  if (i.denominator != 1)
    os << "(" << i.numerator << "/" << i.denominator << ")";
  else
    os << i.numerator;
  return os;
}
}