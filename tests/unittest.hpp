//   Copyright 2022 symxx - caozhanhao
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
#ifndef SYMXX_UNITTEST_HPP
#define SYMXX_UNITTEST_HPP

#include "symxx/symxx.hpp"
#include <random>
#include <map>
#include <set>
#include <functional>
#include <chrono>
#include <experimental/source_location>

namespace symxx::test
{
  class Timer
  {
  private:
    std::chrono::time_point<std::chrono::high_resolution_clock> beg;
  public:
    void start()
    {
      beg = std::chrono::high_resolution_clock::now();
    }
    
    [[nodiscard]] auto get_microseconds() const
    {
      return std::chrono::duration_cast<std::chrono::microseconds>
          (std::chrono::high_resolution_clock::now() - beg).count();
    }
    
    [[nodiscard]] double get_seconds() const
    {
      return static_cast<double>(get_microseconds()) * 0.000001;
    }
  };
  
  int get_edit_distance(const std::string &s1, const std::string &s2)
  {
    std::size_t n = s1.size();
    std::size_t m = s2.size();
    if (n * m == 0) return static_cast<int>(n + m);
    std::vector<std::vector<int>> D(n + 1, std::vector<int>(m + 1));
    for (int i = 0; i < n + 1; i++)
    {
      D[i][0] = i;
    }
    for (int j = 0; j < m + 1; j++)
    {
      D[0][j] = j;
    }
    
    for (int i = 1; i < n + 1; i++)
    {
      for (int j = 1; j < m + 1; j++)
      {
        int left = D[i - 1][j] + 1;
        int down = D[i][j - 1] + 1;
        int left_down = D[i - 1][j - 1];
        if (s1[i - 1] != s2[j - 1]) left_down += 1;
        D[i][j] = std::min(left, std::min(down, left_down));
      }
    }
    return D[n][m];
  }
  
  template<typename T>
  T random_digit(T a, T b)//[a,b]
  {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<T> dis{a, b};
    return dis(gen);
  }
  
  char randichar(int a, int b)//[a,b]
  {
    return static_cast<char>(static_cast<char>(random_digit(a, b)) + '0');
  }
  
  std::string rand_digit_str(size_t n)
  {
    if (n == 0)return "";
    std::string num;
    num += randichar(1, 9);
    for (size_t i = 1; i < n; ++i)
    {
      num += randichar(0, 9);
    }
    return num;
  }
  
  template<typename T>
  std::string to_str(T a)
  {
    return std::to_string(a);
  }
  
  template<>
  std::string to_str(const char *a)
  {
    return {a};
  }
  
  template<>
  std::string to_str(std::string a)
  {
    return a;
  }
  
  template<>
  std::string to_str(bool a)
  {
    return a ? "true" : "false";
  }
  
  template<>
  std::string to_str(const Huge &a)
  {
    return a.to_string();
  }
  
  class Test
  {
  private:
    size_t success;
    std::map<const std::string, const std::function<void(Test &)>> tests_adders;
    std::vector<std::function<void()>> enabled_tests;
    std::set<size_t> exceptions;
    double time;
  public:
    Test() : success(0), time(-1) {}
    
    void add_test_adder(const std::string &name, const std::function<void(Test &)> &f)
    {
      tests_adders.insert({name, f});
    }
    
    template<typename T1, typename T2>
    void add_expected_eq(const std::string &err_msg, const T1 &t1, const T2 &t2,
                         const std::experimental::source_location &l =
                         std::experimental::source_location::current())
    {
      enabled_tests.template emplace_back(
          [this, t1, t2, err_msg, pos = enabled_tests.size(),
              location = std::string(l.file_name()) + ":"
                         + std::to_string(l.line()) +
                         ":" + l.function_name() + "()"]()
          {
            if (t1 != t2)
            {
              std::cerr << ("\033[0;32;31m[FAIL]\033[m Test " + std::to_string(pos) + " in " + location
                            + ": " + err_msg + "[" + to_str(t1) + "\033[0;32;31m != \033[m" + to_str(t2) + "].")
                        << std::endl;
            }
            else
            {
              ++success;
            }
          });
    }
    
    void add_test_func(const std::string &err_msg, std::function<std::tuple<int, std::string>()> func,
                       const std::experimental::source_location &l =
                       std::experimental::source_location::current())
    {
      enabled_tests.template emplace_back(
          [this, func, err_msg, pos = enabled_tests.size(),
              location = std::string(l.file_name()) + ":"
                         + std::to_string(l.line()) +
                         ":" + l.function_name() + "()"]()
          {
            auto ret = func();
            if (std::get<0>(ret) != 0)
            {
              std::cerr << ("\033[0;32;31m[FAIL]\033[m Test " + std::to_string(pos) + " in " + location
                            + ": " + err_msg + "[Function Returns \033[0;32;31m"
                            + std::to_string(std::get<0>(ret)) + "\033[m," + std::get<1>(ret) + "]") << std::endl;
            }
            else
            {
              ++success;
            }
          });
    }
    
    void enable_all_tests()
    {
      for (auto &adder: tests_adders)
      {
        adder.second(*this);
      }
    }
    
    void enable_a_test(const std::string &test_name)
    {
      auto it = tests_adders.find(test_name);
      if (it == tests_adders.end())
      {
        int min_distance = std::numeric_limits<int>::max();
        std::string guess;
        for (auto &r: tests_adders)
        {
          if (get_edit_distance(r.first, test_name) < min_distance)
          {
            guess = r.first;
          }
        }
        throw Error("Unknown test named '" + test_name + "'.Did you mean '" + guess + "'?\n");
      }
      it->second(*this);
    }
    
    void run_tests()
    {
      Timer timer;
      timer.start();
      for (size_t i = 0; i < enabled_tests.size(); ++i)
      {
        try
        {
          enabled_tests[i]();
        }
        catch (Error &e)
        {
          std::cerr << ("\033[0;32;31m[ERROR]\033[m Test " +
                        std::to_string(i) + ": " + e.get_content())
                    << std::endl;
          exceptions.insert(i);
        }
      }
      time = timer.get_seconds();
    }
    
    void print_results()
    {
      if (success == enabled_tests.size())
      {
        std::cout << ("\033[0;32;32mUNITTEST PASSED\033[m: All "
                      + std::to_string(enabled_tests.size()) + " tests passed.") << std::endl;
      }
      else
      {
        std::cout << ("\033[0;32;31mUNITTEST FAILED\033[m: All "
                      + std::to_string(enabled_tests.size()) + " tests, "
                      + std::to_string(enabled_tests.size() - success) + " tests failed.") << std::endl;
      }
      if (!exceptions.empty())
      {
        std::cout << (std::to_string(exceptions.size()) + " symxx::Error was captured[");
        std::string str;
        for (auto &r: exceptions)
        {
          str += std::to_string(r) + ',';
        }
        str.pop_back();
        std::cout << str << "]." << std::endl;
      }
      std::cout << ("Time: " + ::symxx::dtoa(time) + "s.") << std::endl;
    }
  };
}
#endif
