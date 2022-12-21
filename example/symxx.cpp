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
int main()
{
  std::map<std::string, std::tuple<std::vector<std::string>, ExprNode<IntType>>> funcs;
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
        body = str;
      }
      if (cmd == "reduce")
      {
        ExprParser<IntType> p{body};
        auto a = p.parse();
        a.reduce();
        std::cout << a << std::endl;
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
        std::cout << "Function: " << name << "(" << argstr << ") = " << func << std::endl;
        funcs[name] = {args, func};
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
          std::string temp;
          std::vector<Real<IntType>> args;
          size_t i = lp + 1;
          for (; i < rp; i++)
          {
            if (std::isspace(body[i]))
              continue;
            if (body[i] == ',')
            {
              if (temp.empty())
                throw Error(SYMXX_ERROR_LOCATION, __func__, "Argument can not be \"\".");
              args.emplace_back(Real<IntType>{Rational<IntType>{temp}});
              temp = "";
              continue;
            }
            temp += body[i];
          }
          if (!temp.empty())
            args.emplace_back(Real<IntType>{Rational<IntType>{temp}});
          auto fargs = std::get<0>(it->second);
          if (args.size() != fargs.size())
            throw Error(SYMXX_ERROR_LOCATION, __func__, "Expected " + std::to_string(fargs.size()) + " arguments");
          std::map<std::string, Real<IntType>> env;
          for (size_t i = 0; i < fargs.size(); i++)
            env[fargs[i]] = args[i];
          std::cout << std::get<1>(it->second).set_var(env) << std::endl;
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