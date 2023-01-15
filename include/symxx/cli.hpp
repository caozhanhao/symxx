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
#ifndef SYMXX_CLI_HPP
#define SYMXX_CLI_HPP

#include "dtoa.hpp"
#include "error.hpp"
#include "expr.hpp"
#include "factorize.hpp"
#include "frac.hpp"
#include "huge.hpp"
#include "int_adapter.hpp"
#include "num.hpp"
#include "parser.hpp"
#include "utils.hpp"
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <cstdint>
#include <numbers>

namespace symxx
{
  template<typename IntType>
  class BasicCLI
  {
  private:
    static constexpr std::string_view IntTypename = utils::nameof<IntType>();
    std::map<std::string, std::tuple<std::vector<std::string>, ExprNode<IntType>>> funcs
        {
            {"fib", {{"n"}, ExprParser<IntType>{"((1/5)^0.5)*(((1+5^0.5)/2)^n-((1-5^0.5)/2)^n)"}.parse().normalize()}}
        };
    std::map<std::string, long double> constants
        {
            {"pi",     std::numbers::pi},
            {"e",      std::numbers::e},
            {"phi",    std::numbers::phi},
            {"egamma", std::numbers::egamma}
        };
  public:
    void print_func(const std::string &name) const
    {
      const auto &func = funcs.at(name);
      std::string argstr;
      for (auto &a: std::get<0>(func))
      {
        argstr += a + ",";
      }
      if (!argstr.empty())
      {
        argstr.pop_back();
      }
      std::cout << "Function: " << name << "(" << argstr << ") = "
                << std::get<1>(func) << std::endl;
    }
    
    void print_result(ExprNode<IntType> expr) const
    {
      std::cout << expr.normalize();
      if (auto fp = expr.try_eval(); fp != nullptr && fp->no_symbols())
      {
        if (fp->is_rational() && fp->template to<Rational<IntType>>().is_int())
        {
          std::cout << std::endl;
          return;
        }
      }
      
      if (auto ldp = expr.try_eval(constants); ldp != nullptr)
      {
        std::cout << " = " << *ldp << std::endl;
      }
      else
      {
        std::cout << std::endl;
      }
    }
    
    void print_constant(const std::string &name) const
    {
      std::cout << "Constant: " << name << " = " << constants.at(name) << std::endl;
    }
    
    //cmd
    void cmd_normalize(const std::string &body) const
    {
      print_result(ExprParser<IntType>(body).parse().normalize());
    }
    
    void cmd_factor(const std::string &body) const
    {
      std::multiset<IntType> factors;
      auto npp = ExprParser<IntType>(body).parse().normalize().try_eval();
      symxx_assert(npp != nullptr, "Invailed string");
      auto np = npp->try_eval();
      symxx_assert(np != nullptr, "Invailed string");
      auto n = np->template try_to<IntType>();
      symxx_assert(n != nullptr, "Invailed string");
      factorize<IntType>(*n, factors);
      for (auto &r: factors)
      {
        std::cout << adapter_to_string(r) << " ";
      }
      std::cout << std::endl;
    }
    
    void cmd_func(const std::string &body)
    {
      auto lp = body.find_first_of("(");
      auto rp = body.find_first_of(")", lp);
      symxx_assert(lp != std::string::npos && rp != std::string::npos, "Function needs '(' and ')'.");
      auto name = body.substr(0, lp);
      std::string temp;
      std::vector<std::string> args;
      size_t i = lp + 1;
      for (; i < rp; i++)
      {
        if (std::isspace(body[i])) continue;
        if (body[i] == ',')
        {
          symxx_assert(!temp.empty(), "Argument can not be \"\".");
          args.emplace_back(temp);
          temp = "";
          continue;
        }
        temp += body[i];
      }
      if (!temp.empty())
      {
        args.emplace_back(temp);
      }
      ++i; // skip )
      while (std::isspace(body[i])) ++i;
      
      symxx_assert(body[i] == '=', "Expected '='.");
      ExprParser<IntType> p{body.substr(i + 1)};
      auto func = p.parse();
      func.normalize();
      std::string argstr;
      for (auto &a: args)
      {
        argstr += a + ",";
      }
      if (!argstr.empty())
      {
        argstr.pop_back();
      }
      funcs[name] = {args, func};
      print_func(name);
    }
    
    void cmd_constant(const std::string &body)
    {
      auto eq = body.find_first_of("=");
      symxx_assert(eq != std::string::npos, "Expected '='.");
      size_t i = 0;
      std::string temp;
      for (; i < eq; i++)
      {
        if (std::isspace(body[i])) continue;
        temp += body[i];
      }
      symxx_assert(!temp.empty(), "Constant's name can not be empty.");
      constants[temp] = std::stold(body.substr(i + 1));
      print_constant(temp);
    }
    
    void cmd_print(const std::string &body) const
    {
      if (body.empty())
      {
        for (auto &r: funcs)
        {
          print_func(r.first);
        }
        for (auto &r: constants)
        {
          print_constant(r.first);
        }
      }
      else
      {
        auto itf = funcs.find(body);
        auto itv = constants.find(body);
        if (itf != funcs.end())
        {
          print_func(body);
        }
        if (itv != constants.end())
        {
          print_constant(body);
        }
      }
    }
    
    void cmd_call(const std::string &name, const std::string &argstr) const
    {
      std::string temp;
      std::vector<Real<IntType>> args;
      for (auto &i: argstr)
      {
        if (std::isspace(i)) continue;
        if (i == ',')
        {
          symxx_assert(!temp.empty(), "Argument can not be \"\".");
          args.emplace_back(Real<IntType>{Rational<IntType>{temp}});
          temp = "";
          continue;
        }
        temp += i;
      }
      if (!temp.empty())
      {
        args.emplace_back(Real<IntType>{Rational<IntType>{temp}});
      }
      
      auto func = funcs.at(name);
      auto fargs = std::get<0>(func);
      symxx_assert(args.size() == fargs.size(), "Expected " + std::to_string(fargs.size()) + " arguments");
      Environment<IntType> env{std::make_shared<std::map<std::string, Real<IntType>>>()};
      for (size_t i = 0; i < fargs.size(); i++)
      {
        (*env)[fargs[i]] = args[i];
      }
      print_result(std::get<1>(func).substitute(env));
    }
    
    void cmd_version() const
    {
      std::cout << "symxx | version - " << SYMXX_VERSION << " | int - " << IntTypename << std::endl;
    }
  
  public:
    int mainloop()
    {
      cmd_version();
      std::string str, cmd, body;
      while (true)
      {
        try
        {
          std::cout << "symxx> ";
          std::getline(std::cin, str);
          auto pos = str.find_first_of(" ");
          if (pos != std::string::npos)
          {
            cmd = str.substr(0, pos);
            body = str.substr(pos + 1);
          }
          else
          {
            cmd = str;
            body = "";
          }
          if (cmd == "normalize") { cmd_normalize(body); }
          else if (cmd == "func") { cmd_func(body); }
          else if (cmd == "constant") { cmd_constant(body); }
          else if (cmd == "print") { cmd_print(body); }
          else if (cmd == "factor") { cmd_factor(body); }
          else if (cmd == "version") { cmd_version(); }
          else if (cmd == "quit") { return 0; }
          else
          {
            auto lp = cmd.find_first_of("(");
            auto rp = cmd.find_first_of(")", lp);
            if (lp == std::string::npos || rp == std::string::npos)
            {
              cmd_normalize(str);
              continue;
            }
            auto it = funcs.find(cmd.substr(0, lp));
            if (it == funcs.end())
            {
              cmd_normalize(str);
              continue;
            }
            else
            {
              cmd_call(it->first, cmd.substr(lp + 1, rp - lp - 1));
            }
          }
        }
        catch (Error &e)
        {
          std::cerr << e.get_content() << std::endl;
        }
      }
      return 0;
    }
  };

#if defined(SYMXX_ENABLE_INT128)
  using CLI = BasicCLI<__int128_t>;
#elif define(SYMXX_ENABLE_HUGE)
  using CLI = BasicCLI<Huge>;
#else
  using CLI = BasicCLI<int64_t>;
#endif

}
#endif