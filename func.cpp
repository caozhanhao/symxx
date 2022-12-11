#include "expr.hpp"
#include <iostream>
int main() {
  expr::Frac<int> func{{{2}}, {{1, {"x"}}, {1}}};
  std::cout << "f(x) = " << func << std::endl;
  std::string input;
  num::Rational<int> i;  
  while (true) {
    std::cout << ">> ";
    std::cin >> input;
    try {
      i = num::Rational<int>{input};
    } catch (...) {
      std::cout << "Input Error." << std::endl;
      continue;
    }
    std::cout << "f(" << i << ") = " << func.set_var({{"x", i}}) << std::endl;
  }
  return 0;
}
