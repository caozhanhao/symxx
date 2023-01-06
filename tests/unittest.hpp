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
#ifndef SYMXX_UNITTEST_HPP
#define SYMXX_UNITTEST_HPP

#include "symxx/symxx.hpp"
#include <random>
#include <map>
#include <set>
#include <functional>
#include <chrono>
#include <type_traits>
#include <experimental/source_location>

#define _SYMXX_LAZY_EVAL(x) [=](){return (x);}
#define SYMXX_EXPECT_EQ(msg, v1, v2) test::get_test().expected_eq((msg), _SYMXX_LAZY_EVAL((v1)), _SYMXX_LAZY_EVAL((v2)));
#define SYMXX_EXPECT_TRUE(msg, v1) test::get_test().expected_eq((msg), _SYMXX_LAZY_EVAL((v1)), _SYMXX_LAZY_EVAL(true));
#define SYMXX_EXPECT_FALSE(msg, v1) test::get_test().expected_eq((msg), _SYMXX_LAZY_EVAL((v1)), _SYMXX_LAZY_EVAL(false));
#define SYMXX_EXPECT_SUCCESS(msg, f) test::get_test().expected_success((msg), f);
#define SYMXX_MARK(msg) test::get_test().mark(msg);

namespace symxx::test
{
  class Test;
  
  Test &get_test();
  
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
  
  namespace test_internal
  {
    struct StrTag {};
    struct SymxxTag {};
    struct BasicTag {};
    struct BoolTag {};
    struct CharptrTag {};
    template<typename U>
    struct TagDispatch
    {
      using tag = std::conditional_t<std::is_same_v<std::decay_t<U>, std::string>, StrTag,
          std::conditional_t<std::is_same_v<std::decay_t<U>, const char *>, CharptrTag,
              std::conditional_t<std::is_same_v<std::decay_t<U>, bool>, BoolTag,
                  std::conditional_t<std::is_fundamental_v<std::decay_t<U>>, BasicTag, SymxxTag>>>>;
    };
    
    std::string internal_to_str(const char *a, CharptrTag)
    {
      return {a};
    }
    
    std::string internal_to_str(std::string a, StrTag)
    {
      return a;
    }
    
    std::string internal_to_str(bool a, BoolTag)
    {
      return a ? "true" : "false";
    }
    
    template<typename T>
    std::string internal_to_str(const T &a, SymxxTag)
    {
      return a.to_string();
    }
    
    template<typename T>
    std::string internal_to_str(const T &a, BasicTag)
    {
      return std::to_string(a);
    }
  }
  
  template<typename T>
  std::string to_str(T &&a)
  {
    return test_internal::internal_to_str(a, typename test_internal::TagDispatch<std::decay_t<T>>::tag{});
  }
  
  class Test
  {
  private:
    size_t success;
    std::vector<std::function<void()>> all_tests;
    std::vector<size_t> exceptions;
    std::vector<size_t> failure;
    std::vector<std::pair<size_t, std::string>> marks;
    double time;
    std::string results;
  public:
    Test() : success(0), time(0) {}
    
    void mark(const std::string &msg)
    {
      marks.emplace_back(std::make_pair(all_tests.size(), msg));
    }
    
    template<typename T1, typename T2>
    void expected_eq(const std::string &msg, const T1 &t1, const T2 &t2,
                     const std::experimental::source_location &l =
                     std::experimental::source_location::current())
    {
      all_tests.template emplace_back(
          [this, t1, t2, msg, pos = all_tests.size(),
              location = std::string(l.file_name()) + ":"
                         + std::to_string(l.line()) +
                         ":" + l.function_name() + "()"]()
          {
            auto v1 = t1();
            auto v2 = t2();
            if (v1 != v2)
            {
              results += ("[\033[0;32;31mFAILED\033[m] Test " + std::to_string(pos) + " in " + location
                          + ": " + msg + "[" + to_str(v1) + "\033[0;32;31m != \033[m" + to_str(v2) + "].\n");
              failure.emplace_back(pos);
            }
            else
            {
              ++success;
            }
          });
    }
    
    void expected_success(const std::string &msg, std::function<std::tuple<int, std::string>()> func,
                          const std::experimental::source_location &l =
                          std::experimental::source_location::current())
    {
      all_tests.template emplace_back(
          [this, func, msg, pos = all_tests.size(),
              location = std::string(l.file_name()) + ":"
                         + std::to_string(l.line()) +
                         ":" + l.function_name() + "()"]()
          {
            auto ret = func();
            if (std::get<0>(ret) != 0)
            {
              results += ("[\033[0;32;31mFAILED\033[m] Test " + std::to_string(pos) + " in " + location
                          + ": " + msg + "[Function Returns \033[0;32;31m"
                          + std::to_string(std::get<0>(ret)) + "\033[m," + std::get<1>(ret) + "]\n");
              failure.emplace_back(pos);
            }
            else
            {
              ++success;
            }
          });
    }
    
    void run_tests()
    {
      SYMXX_MARK("END");
      Timer timer;
      timer.start();
      std::string curr_msg;
      size_t curr_pos = 0;
      bool print_mark = false;
      auto s = timer.get_microseconds();
      auto e = s;
      for (size_t i = 0; i < all_tests.size(); ++i)
      {
        if (curr_pos + 1 < marks.size() && (i >= marks[curr_pos + 1].first || i + 1 == all_tests.size()))
        {
          curr_msg = marks[curr_pos].second;
          ++curr_pos;
          print_mark = true;
        }
        try
        {
          all_tests[i]();
          e = timer.get_microseconds();
        }
        catch (Error &e)
        {
          results += ("[\033[0;32;31mEXCEPTION\033[m] Test " +
                      std::to_string(i) + " | " + e.get_content() + "\n");
          exceptions.emplace_back(i);
        }
        if (print_mark && !curr_msg.empty())
        {
          size_t beg = (curr_pos <= 1 ? 0 : marks[curr_pos - 1].first);
          size_t end = marks[curr_pos].first;
          if ((failure.empty() || failure.back() >= end || failure.back() < beg)
              && (exceptions.empty() || exceptions.back() >= end || exceptions.back() < beg))
          {
            size_t ntests = end - beg;
            results += "[\033[0;32;32mPASSED\033[m] " + curr_msg
                       + " | " + std::to_string(ntests) + " tests passed."
                       + "(" + ::symxx::dtoa((e - s) * 0.001) + " ms)\n";
          }
          print_mark = false;
          time += e - s;
          s = timer.get_microseconds();
        }
      }
    }
    
    void print_results()
    {
      std::cout << results;
      if (success == all_tests.size())
      {
        std::cout << ("\033[0;32;32mUNITTEST PASSED\033[m | All "
                      + std::to_string(all_tests.size()) + " tests passed.");
      }
      else
      {
        std::cout << ("\033[0;32;31mUNITTEST FAILED\033[m | Of all "
                      + std::to_string(all_tests.size()) + " tests, "
                      + std::to_string(all_tests.size() - success) + " tests failed.");
      }
      std::cout << ("(" + ::symxx::dtoa(time * 0.000001) + " s)\n");
      if (!exceptions.empty())
      {
        std::cout << "[\033[0;32;33mDETAIL\033[m]" << (std::to_string(exceptions.size()) + " exceptions at[");
        std::string str;
        for (auto &r: exceptions)
        {
          str += std::to_string(r) + ',';
        }
        str.pop_back();
        std::cout << str << "]." << std::endl;
      }
      if (!failure.empty())
      {
        std::cout << "[\033[0;32;33mDETAIL\033[m]" << (std::to_string(failure.size()) + " failure at[");
        std::string str;
        for (auto &r: failure)
        {
          str += std::to_string(r) + ',';
        }
        str.pop_back();
        std::cout << str << "]." << std::endl;
      }
    }
  };
  
  Test &get_test()
  {
    static Test test;
    return test;
  }
}
#endif
