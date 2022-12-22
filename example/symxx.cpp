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
#include "symxx/dtoa.hpp"
#include "symxx/expr.hpp"
#include "symxx/num.hpp"
#include "symxx/utils.hpp"
#include "symxx/parser.hpp"
#include <iostream>
#include <map>
#include <string>
#include <tuple>
using namespace symxx;
using IntType = long long int;
void print_func(const std::string &name, const std::tuple<std::vector<std::string>, ExprNode<IntType>> &func)
{
  std::string argstr;
  for (auto &a : std::get<0>(func))
    argstr += a + ",";
  if (!argstr.empty())
    argstr.pop_back();
  std::cout << "Function: " << name << "(" << argstr << ") = "
            << std::get<1>(func) << std::endl;
}
void print_result(const ExprNode<IntType> &expr)
{
  auto fp = expr.try_eval();
  if (fp != nullptr && fp->no_symbols())
  {
    if (!fp->is_rational())
      std::cout << expr << " ≈ " << ::symxx::dtoa(fp->template to<double>()) << std::endl;
    else if (fp->template to<Rational<IntType>>().get_denominator() != 1)
      std::cout << expr << " = " << ::symxx::dtoa(fp->template to<double>()) << std::endl;
    else
      std::cout << *fp << std::endl;
    return;
  }
  std::cout << expr << std::endl;
}
void print_var(const std::string &name, const Real<IntType> &var)
{
  std::cout << "Variable: " << name << " = ";
  print_result(Frac<IntType>{var});
}
int main()
{
  std::map<std::string, std::tuple<std::vector<std::string>, ExprNode<IntType>>> funcs;
  std::map<std::string, Real<IntType>> vars{
      {"pi", static_cast<long double>(3.14159265358979311)},
      {"e", static_cast<long double>(2.718281828459045235)},
  };
  std::string str, cmd, body;
  while (true)
  {
    try
    {
      std::cout << ">> ";
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
      if (cmd == "reduce")
      {
        ExprParser<IntType> p{body};
        auto a = p.parse();
        std::map<std::string, Real<IntType>> env;
        for (auto &r : vars)
          env[r.first] = r.second;
        print_result(a.set_var(env));
      }
      else if (cmd == "func")
      {
        auto lp = body.find_first_of("(");
        auto rp = body.find_first_of(")", lp);
        if (lp == std::string::npos || rp == std::string::npos)
          throw Error(SYMXX_ERROR_LOCATION, __func__, "Input error.");
        auto name = body.substr(0, lp);
        std::string temp;
        std::vector<std::string> args;
        size_t i = lp + 1;
        for (; i < rp; i++)
        {
          if (std::isspace(body[i]))
            continue;
          if (body[i] == ',')
          {
            if (temp.empty())
              throw Error(SYMXX_ERROR_LOCATION, __func__, "Argument can not be \"\".");
            args.emplace_back(temp);
            temp = "";
            continue;
          }
          temp += body[i];
        }
        if (!temp.empty())
          args.emplace_back(temp);
        ++i; // skip )
        while (std::isspace(body[i]))
          ++i;
        if (body[i] != '=')
          throw Error(SYMXX_ERROR_LOCATION, __func__, "Expected '='.");
        ExprParser<IntType> p{body.substr(i + 1)};
        auto func = p.parse();
        func.reduce();
        std::string argstr;
        for (auto &a : args)
          argstr += a + ",";
        if (!argstr.empty())
          argstr.pop_back();
        funcs[name] = {args, func};
        print_func(name, funcs[name]);
      }
      else if (cmd == "var")
      {
        auto eq = body.find_first_of("=");
        if (eq == std::string::npos)
          throw Error(SYMXX_ERROR_LOCATION, __func__, "Expected '='.");
        size_t i = 0;
        std::string temp;
        for (; i < eq; i++)
        {
          if (std::isspace(body[i]))
            continue;
          temp += body[i];
        }
        if (temp.empty())
          throw Error(SYMXX_ERROR_LOCATION, __func__, "Variable's name can not be empty.");
        vars[temp] = Real<IntType>{Rational<IntType>{body.substr(i + 1)}};
        print_var(temp, vars[temp]);
      }
      else if (cmd == "print")
      {
        if (body.empty())
        {
          for (auto &r : funcs)
            print_func(r.first, r.second);
          for (auto &r : vars)
            print_var(r.first, r.second);
        }
        else
        {
          auto itf = funcs.find(body);
          auto itv = vars.find(body);
          if (itf != funcs.end())
            print_func(body, itf->second);
          if (itv != vars.end())
            print_var(body, itv->second);
        }
      }
      else if (cmd == "quit")
        return 0;
      else
      {
        auto lp = cmd.find_first_of("(");
        auto rp = cmd.find_first_of(")", lp);
        if (lp == std::string::npos || rp == std::string::npos)
          throw Error(SYMXX_ERROR_LOCATION, __func__, "Unknown command.");
        auto it = funcs.find(cmd.substr(0, lp));
        if (it == funcs.end())
          throw Error(SYMXX_ERROR_LOCATION, __func__, "Unknown command.");
        else
        {
          auto get_real = [&vars](const std::string &str) -> Real<IntType>
          {
            auto it = vars.find(str);
            if (it == vars.end())
              return Real<IntType>{Rational<IntType>{str}};
            return it->second;
          };
          std::string temp;
          std::vector<Real<IntType>> args;
          size_t i = lp + 1;
          for (; i < rp; i++)
          {
            if (std::isspace(cmd[i]))
              continue;
            if (cmd[i] == ',')
            {
              if (temp.empty())
                throw Error(SYMXX_ERROR_LOCATION, __func__, "Argument can not be \"\".");
              args.emplace_back(get_real(temp));
              temp = "";
              continue;
            }
            temp += cmd[i];
          }
          if (!temp.empty())
            args.emplace_back(get_real(temp));
          auto fargs = std::get<0>(it->second);
          if (args.size() != fargs.size())
            throw Error(SYMXX_ERROR_LOCATION, __func__, "Expected " + std::to_string(fargs.size()) + " arguments");
          std::map<std::string, Real<IntType>> env;
          for (auto &r : vars)
            env[r.first] = r.second;
          for (size_t i = 0; i < fargs.size(); i++)
            env[fargs[i]] = args[i];
          print_result(std::get<1>(it->second).set_var(env));
        }
      }
    }
    catch (Error &e)
    {
      std::cout << e.get_content() << std::endl;
    }
  }
  return 0;
}