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
#ifndef SYMXX_PARSER_HPP
#define SYMXX_PARSER_HPP
#include "num.hpp"
#include "frac.hpp"
#include "expr.hpp"
#include "error.hpp"
#include <string>
#include <stack>
#include <variant>
#include <map>

namespace symxx
{
  enum class ExprTokenType
  {
    BEGIN,
    END,
    RADICAL,
    OP,
    LPAREN,
    RPAREN,
    DIGIT,
    SYMBOL
  };
  
  template<typename T>
  class ExprToken
  {
  private:
    std::variant<char, Frac < T>, Rational <T>> val;
    ExprTokenType ExprToken_type;

  public:
    template<typename U>
    ExprToken(ExprTokenType t, U &&u)
        : val(std::forward<U>(u)), ExprToken_type(t) {}
  
    auto type() const { return ExprToken_type; }
  
    auto &frac() const
    {
      return std::get<Frac < T>>
      (val);
    }
  
    auto &ch() const { return std::get<char>(val); }
  
    auto &rational() const
    {
      return std::get<Rational < T>>
      (val);
    }
  };
  
  template<typename T>
  class ExprParser
  {
  private:
    std::string raw;
    std::vector<ExprToken<T>> tokens;
    std::size_t rawpos;
    std::size_t tokenpos;
  public:
    ExprParser(const std::string &r)
        : raw(r), rawpos(0), tokenpos(1) {}
  
    ExprNode <T> parse()
    {
      get_all_tokens();
      std::stack<ExprNode<T> *> nodes;
      std::stack<char> op;
      auto handle = [&nodes, &op]()
      {
        symxx_assert(nodes.size() >= 2 && !op.empty(), "Invaild string.");
        auto *rhs = nodes.top();
        nodes.pop();
        auto *lhs = nodes.top();
        nodes.pop();
        nodes.emplace(new ExprNode<T>(op.top(), lhs, rhs));
        op.pop();
      };
      for (tokenpos = 1; tokenpos + 1 < tokens.size(); ++tokenpos)
      {
        auto &curr = tokens[tokenpos];
        if (curr.type() == ExprTokenType::OP)
        {
          while (!op.empty() && great(op.top(), curr.ch()))
          {
            handle();
          }
          op.push(curr.ch());
          continue;
        }
        else if (curr.type() == ExprTokenType::LPAREN)
        {
          op.push('(');
          continue;
        }
        else if (curr.type() == ExprTokenType::RPAREN)
        {
          symxx_assert(!op.empty(), "Expected '('");
          while (!nodes.empty() && op.top() != '(')
          {
            handle();
            symxx_assert(!op.empty(), "Expected '('");
          }
          symxx_assert(!op.empty(), "Invaild string.");
          op.pop();
          continue;
        }
        else if (curr.type() == ExprTokenType::DIGIT || curr.type() == ExprTokenType::SYMBOL)
        {
          nodes.emplace(new ExprNode<T>(curr.frac()));
          continue;
        }
        else
        {
          symxx_unreachable("unexpected ExprToken.");
        }
      }
      while (!op.empty())
      {
        handle();
      }
      symxx_assert(nodes.size() == 1, "Invaild string.");
      symxx_assert(op.empty(), "Invaild string.");
      return *nodes.top();
    }

  private:
    void get_all_tokens()
    {
      tokens.template emplace_back(ExprToken<T>{ExprTokenType::BEGIN, 0});
      while (tokens.back().type() != ExprTokenType::END)
      {
        auto token = get_token();
        if ((token.type() == ExprTokenType::SYMBOL || token.type() == ExprTokenType::LPAREN)
            && (tokens.back().type() == ExprTokenType::SYMBOL //xx -> x*x
                || tokens.back().type() == ExprTokenType::DIGIT  //2x -> 2*x
                || tokens.back().type() == ExprTokenType::RPAREN))//(1+1)x -> (1+1)*x
        {
          tokens.template emplace_back(ExprToken<T>{ExprTokenType::OP, '*'});
          tokens.template emplace_back(token);
          continue;
        }
        if (tokens.back().type() == ExprTokenType::RADICAL)
          // _/2 -> 2**(1/2), _3/2 -> 2**(1/3)
        {
          symxx_assert(token.type() == ExprTokenType::DIGIT, "Radical needs a rational.");
          auto power = tokens.back().rational();
          tokens.pop_back();
          tokens.template emplace_back(ExprToken<T>{ExprTokenType::LPAREN, '('});
          tokens.template emplace_back(token);
          tokens.template emplace_back(ExprToken<T>{ExprTokenType::OP, '^'});
          tokens.template emplace_back(ExprToken<T>{ExprTokenType::DIGIT,
                                                    Frac<T>{power}});
          tokens.template emplace_back(ExprToken<T>{ExprTokenType::RPAREN, ')'});
          continue;
        }
        tokens.template emplace_back(token);
      }
    }
  
    Rational <T> parse_a_number()
    {
      bool has_dot = raw[rawpos] == '.';
      std::string temp;
      do
      {
        temp += raw[rawpos];
        ++rawpos;
      } while (rawpos < raw.size() && (std::isdigit(raw[rawpos]) ||
                                       (has_dot ? false : raw[rawpos] == '.')));
      if (temp == "+" || temp == "-") temp += '1';
      return Rational<T>{temp};
    }
  
    ExprToken<T> get_token()
    {
      while (rawpos < raw.size() && std::isspace(raw[rawpos]))
      {
        ++rawpos;
      }
      if (rawpos >= raw.size())
      {
        return {ExprTokenType::END, 0};
      }
      auto &ch = raw[rawpos];
      if (ch == '_')
      {
        ++rawpos;
        Rational<T> power;
        if (raw[rawpos] == '/')
        {
          power = 2;
        }
        else if (std::isdigit(raw[rawpos]) || raw[rawpos] == '.')
        {
          power = parse_a_number();
        }
        else
        {
          symxx_unreachable("Unexpected '_', needs a '/'");
        }
        ++rawpos;
        return {ExprTokenType::RADICAL, power.inverse()};
      }
      else if (std::isdigit(ch) || ch == '.'
               || (tokens.size() == 1 && (ch == '-' || ch == '+')))//'+' or '-' at begin
      {
        return {ExprTokenType::DIGIT, Frac<T>{parse_a_number()}};
      }
      else if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '^')
      {
        if (ch == '*' && rawpos + 1 < raw.size() && raw[rawpos + 1] == '*')
        {
          rawpos += 2;
          return {ExprTokenType::OP, '^'};
        }
        else
        {
          ++rawpos;
          return {ExprTokenType::OP, ch};
        }
      }
      else if (ch == '(')
      {
        ++rawpos;
        return {ExprTokenType::LPAREN, '('};
      }
      else if (ch == ')')
      {
        ++rawpos;
        return {ExprTokenType::RPAREN, ')'};
      }
      else if (std::isalpha(ch) || ch == '{')
      {
        std::string symbol;
        if (ch == '{')
        {
          ++rawpos;//skip '{'
          while (rawpos < raw.size() && raw[rawpos] != '}')
          {
            symbol += raw[rawpos];
            ++rawpos;
          }
        }
        else
        {
          symbol += ch;
        }
        ++rawpos;//skip a single char symbol or '}'
        return {ExprTokenType::SYMBOL, Frac<T>{{Term<T>{Real<T>{1}, symbol}}}};
      }
      symxx_unreachable("unexpected '" + std::string(1, ch) + "'.");
      return {ExprTokenType::END, 0};
    }
    
    bool great(char a, char b) const
    {
      const static std::map<char, int> priority = {
          {'(', 0},
          {'+', 10},
          {'-', 10},
          {'*', 100},
          {'/', 100},
          {'^', 1000}};;
      return priority.at(a) > priority.at(b);
    }
  };
}
#endif