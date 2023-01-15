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
#ifndef SYMXX_FACTORIZE_HPP
#define SYMXX_FACTORIZE_HPP

#include "error.hpp"
#include "int_adapter.hpp"
#include <string>
#include <vector>
#include <set>
#include <span>
#include <random>

namespace symxx
{
  template<typename T>
  T random_digit(T a, T b)//[a,b]
  {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<T> dis{a, b};
    return dis(gen);
  }
  
  namespace factorize_internal
  {
    // is_prime(), adapted from https://github.com/nishanth17/factor
    // or https://zhuanlan.zhihu.com/p/389061210
    template<typename T>
    bool is_prime_slow_path(T num)
    {
      if (num < 2) return false;
      if (num == 2 || num == 3) return true;
      if (!(num & 1)) return false;
      if (!(num % 3)) return false;
      if (num < 9) return true;
      T sqrt_n = static_cast<T>(adapter_sqrt(num)) + 1;
      for (T i = 5; i < sqrt_n; i += 6)
      {
        if (!(num % i) || !(num % (i + 2)))
        {
          return false;
        }
      }
      return true;
    }
    
    template<typename T>
    bool is_prime_fast_path(T n, bool use_probabilistic = false, bool tolerance = 30)
    {
      const std::vector<int> first_prime = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47,
                                            53, 59, 61, 67, 71};
      std::vector<T> w;

#ifdef SYMXX_ENABLE_HUGE
      static Huge bigint1 = adapter_to_int<Huge>("1000000000000000000000000000000000000");
#endif
#ifdef SYMXX_ENABLE_INT128
      static __int128_t bigint2 = adapter_to_int<__int128_t>("1543267864443420616877677640751301");
      static __int128_t bigint3 = adapter_to_int<__int128_t>("564132928021909221014087501701");
      static __int128_t bigint4 = adapter_to_int<__int128_t>("59276361075595573263446330101");
      static __int128_t bigint5 = adapter_to_int<__int128_t>("6003094289670105800312596501");
      static __int128_t bigint6 = adapter_to_int<__int128_t>("3317044064679887385961981");
      static __int128_t bigint7 = adapter_to_int<__int128_t>("318665857834031151167461");
#endif
      if (false) {}
#ifdef SYMXX_ENABLE_HUGE
        else if (n >= bigint1)
        {
          auto logn = adapter_log<T>(n);
          if (!use_probabilistic)
            w = huge_range(2, 2 * int(logn * adapter_log<T>(logn) / adapter_log<T>(2)));
          else
            w = huge_range(tolerance);
        }
#endif
#ifdef SYMXX_ENABLE_INT128
      else if (n >= bigint2)
      {
        w = {first_prime.begin(), first_prime.begin() + 20};
      }
      else if (n >= bigint3)
      {
        w = {first_prime.begin(), first_prime.begin() + 18};
      }
      else if (n >= bigint4)
      {
        w = {first_prime.begin(), first_prime.begin() + 16};
      }
      else if (n >= bigint5)
      {
        w = {first_prime.begin(), first_prime.begin() + 15};
      }
      else if (n >= bigint6)
      {
        w = {first_prime.begin(), first_prime.begin() + 14};
      }
      else if (n >= bigint7)
      {
        w = {first_prime.begin(), first_prime.begin() + 13};
      }
#endif
      else if (n >= 3825123056546413051)
      {
        w = std::vector<T>{first_prime.begin(), first_prime.begin() + 12};
        // {2, 3, 5, 7, 11, 13, 17, 19, 23}
      }
      else if (n >= 341550071728321)
      {
        w = std::vector<T>{first_prime.begin(), first_prime.begin() + 9};
        // {2, 3, 5, 7, 11, 13, 17}
      }
      else if (n >= 3474749660383)
      {
        w = std::vector<T>{first_prime.begin(), first_prime.begin() + 7};
      }
      else if (n >= 2152302898747)
      {
        w = std::vector<T>{first_prime.begin(), first_prime.begin() + 6};
        // {2, 3, 5, 7, 11, 13}
      }
      else if (n >= 4759123141)
      {
        w = std::vector<T>{first_prime.begin(), first_prime.begin() + 5};
        // {2, 3, 5, 7, 11}
      }
      else if (n >= 9006403)
      {
        w = std::vector<T>{2, 7, 61};
      }
      else if (n >= 489997)
        // Some Fermat stuff
      {
        if (!(!(n & 1) || !(n % 3) || !(n % 5) || !(n % 7) || !(n % 11) || !(n % 13) || !(
            n % 17) || !(n % 19) || !(n % 23) || !(n % 29) || !(n % 31) || !(n % 37) || !(
            n % 41) || !(n % 43) || !(n % 47) || !(n % 53) || !(n % 59) || !(n % 61) || !(
            n % 67) || !(n % 71) || !(n % 73) || !(n % 79) || !(n % 83)) && n % 89 && n % 97 && n % 101)
        {
          T hn = n >> 1;
          T nm1 = n - 1;
          T p = adapter_modpow<T>(static_cast<T>(2), hn, n);
          if (p == 1 || p == nm1)
          {
            p = adapter_modpow<T>(static_cast<T>(3), hn, n);
            if (p == 1 || p == nm1)
            {
              p = adapter_modpow<T>(static_cast<T>(5), hn, n);
              return p == 1 || p == nm1;
            }
          }
        }
        return false;
      }
      else if (n >= 42799)
      {
        return n & 1 && n % 3 && n % 5 && n % 7 && n % 11 && n % 13 && n % 17
               && n % 19 && n % 23 && n % 29 && n % 31 && n % 37 && n % 41 && n % 43
               && adapter_modpow<T>(static_cast<T>(2), n - 1, n) == 1 &&
               adapter_modpow<T>(static_cast<T>(5), n - 1, n) == 1;
      }
      else if (n >= 841)
      {
        return !(((((!(n & 1) || !(n % 3) || !(n % 5) || !(n % 7) || !(n % 11) || !(
            n % 13) || !(
            n % 17)) || !(n % 19)) || !(n % 23) || !(n % 29) || !(n % 31) || !(n % 37) || !(
            n % 41) || !(n % 43) || !(
            n % 47)) || !(n % 53) || !(n % 59) || !(n % 61) || !(n % 67) || !(n % 71) || !(
            n % 73) || !(n % 79)) || !(n % 83) || !(n % 89) || !(n % 97) || !(n % 101) || !(
            n % 103) || !(adapter_modpow<T>(static_cast<T>(2), n - 1, n) == 1));
      }
      else if (n >= 25)
      {
        return !(!(n & 1) || !(n % 3) || !(n % 5) || !(n % 7) || !(n % 11)) && n % 13 && n % 17 && n % 19 && n % 23;
      }
      else if (n >= 4)
      {
        return n & 1 && n % 3;
      }
      else
      {
        return n > 1;
      }
      
      if (!(n & 1 && n % 3 && n % 5 && n % 7 && n % 11 && n % 13 && n % 17 && n % 19 && n % 23 && n % 29 &&
            n % 31 && n % 37 && n % 41 && n % 43 && n % 47 && n % 53 && n % 59 && n % 61 && n % 67 && n % 71 &&
            n % 73 && n % 79 && n % 83 && n % 89))
      {
        return false;
      }
      
      // Miller-Rabin
      T s = 0;
      T d = n - 1;
      while (!(d & 1))
      {
        d >>= 1;
        s += 1;
      }
      for (auto &k: w)
        // Pick a random witness if probabilistic
      {
        T p, x;
        if (use_probabilistic)
        {
          p = random_digit<T>(2, n - 3);
        }
        else
        {
          p = k;
        }
        x = adapter_modpow<T>(p, d, n);
        
        if (x == 1)
        {
          continue;
        }
        else
        {
          bool next = true;
          for (T i = 0; i < s; ++i)
          {
            if (x + 1 == n)
            {
              next = false;
              break;
            }
            x = adapter_mulmod<T>(x, x, n);
          }
          if (next)
          {
            return false;
          }
          else
          {
            break;
          }
        }
      }
      return true;
    }
    
    template<typename T>
    bool is_prime(T n, bool use_probabilistic = false, int tolerance = 30)
    {
#ifdef SYMXX_ENABLE_HUGE
      static T bigint1 = adapter_to_int<T>("1000000000000000000000000000000000000");
#endif
      if (n < 100000)
      {
        return is_prime_slow_path<T>(n);
      }
      else
      {
        if (use_probabilistic)
        {
          return is_prime_fast_path<T>(n, use_probabilistic, tolerance);
        }
        else
        {
#ifdef SYMXX_ENABLE_HUGE
          if (n < bigint1)
            return is_prime_fast_path<T>(n);
          else
            return is_prime_fast_path<T>(n, true, 40);
#else
          return is_prime_fast_path<T>(n);
#endif
        }
      }
      symxx_unreachable();
      return false;
    }
    
    // A C++ implementation of Pollard-Rho,
    // which is adapted from https://zhuanlan.zhihu.com/p/267884783
    template<typename T>
    T Pollard_Rho(const T &num)
    {
      if (num == 4)
      {
        return 2;
      }
      if (is_prime<T>(num))
      {
        return num;
      }
      while (true)
      {
        T c = random_digit<T>(1, num - 2);
        auto f = [&c, &num](const T &x)
        {
          return (adapter_mulmod<T>(x, x, num) + c % num) % num;
          // (x * x + c) % num
        };
        T t = 0, r = 0, p = 1, q;
        do
        {
          for (int i = 0; i < 128; ++i)
          {
            t = f(t), r = f(f(r));
            if (t == r || (q = adapter_mulmod<T>(p, symxx::adapter_abs(t - r), num)) == 0)
            {
              break;
            }
            p = q;
          }
          T d = symxx::adapter_gcd<T>(p, num);
          if (d > 1)
          {
            return d;
          }
        } while (t != r);
      }
      symxx_unreachable();
      return 0;
    }
  }
  
  template<typename T>
  void factorize(T n, std::multiset<T> &ret)
  {
    if (n == 1) return;
    T fac = factorize_internal::Pollard_Rho<T>(n);
    n /= fac;
    
    if (!factorize_internal::is_prime(fac))
    {
      factorize(fac, ret);
    }
    else
    {
      ret.insert(fac);
    }
    if (!factorize_internal::is_prime(n))
    {
      factorize(n, ret);
    }
    else
    {
      ret.insert(n);
    }
  }
}
#endif