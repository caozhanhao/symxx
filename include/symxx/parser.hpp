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
    std::variant<char, Frac < T>> val;
    ExprTokenType ExprToken_type;
  
  public:
    template<typename U>
    ExprToken(ExprTokenType t, U &&u)
        : val(std::forward<U>(u)), ExprToken_type(t) {}
    
    auto type() const { return ExprToken_type; }
    
    auto frac() const
    {
      return std::get<Frac < T>>
      (val);
    }
    
    auto ch() const { return std::get<char>(val); }
  };
  
  template<typename T>
  class ExprParser
  {
  private:
    std::string raw;
    std::size_t pos;
    ExprToken<T> curr;
    bool parsing_end;
    bool last_token_added_a_mul;
    bool parsing_negative;
  
  public:
    ExprParser(const std::string &r)
        : raw(r), pos(0), curr(ExprTokenType::OP, '\0'), parsing_end(false), last_token_added_a_mul(false),
          parsing_negative(false) {}
    
    ExprNode <T> parse()
    {
      pos = 0;
      std::stack<ExprNode<T> *> nodes;
      std::stack<char> op;
      auto handle = [&nodes, &op]()
      {
        if (nodes.size() < 2 || op.empty())
        {
          throw Error("Invaild string.");
        }
        auto *rhs = nodes.top();
        nodes.pop();
        auto *lhs = nodes.top();
        nodes.pop();
        nodes.emplace(new ExprNode<T>(op.top(), lhs, rhs));
        op.pop();
      };
      while (true)
      {
        next();
        if (parsing_end)
          break;
  
        if (curr.type() == ExprTokenType::OP)
        {
          while (!op.empty() && great(op.top(), curr.ch()))
            handle();
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
          if (op.empty())
            throw Error("Expected '('");
          while (!nodes.empty() && op.top() != '(')
          {
            handle();
            if (op.empty())
              throw Error("Expected '('");
          }
          if (op.empty())
            throw Error("Invaild string.");
          op.pop();
          continue;
        }
        else if (curr.type() == ExprTokenType::DIGIT || curr.type() == ExprTokenType::SYMBOL)
        {
          nodes.emplace(new ExprNode<T>(curr.frac()));
          continue;
        }
        else
          throw Error("Unexpected ExprToken.");
      }
      while (!op.empty())
        handle();
      if (nodes.size() != 1)
        throw Error("Invaild string.");
      return *nodes.top();
    }
  
  private:
    void next()
    {
      curr = get_ExprToken();
    }
    
    ExprToken<T> get_ExprToken()
    {
      while (pos < raw.size() && std::isspace(raw[pos]))
        ++pos;
      if (pos >= raw.size())
      {
        parsing_end = true;
        return curr;
      }
      auto &ch = raw[pos];
      if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '^')
      {
        if (ch == '*' && pos + 1 < raw.size() && raw[pos + 1] == '*')
        {
          pos += 2;
          return {ExprTokenType::OP, '^'};
        }
        else if (ch == '-' &&
                 (curr.type() == ExprTokenType::LPAREN || (curr.type() == ExprTokenType::OP && curr.ch() == '\0')))
        {
          parsing_negative = true;
          pos++;
          ch = raw[pos];
        }
        else
        {
          ++pos;
          return {ExprTokenType::OP, ch};
        }
      }
      else if (ch == '(')
      {
        if ((curr.type() == ExprTokenType::SYMBOL || curr.type() == ExprTokenType::DIGIT ||
             curr.type() == ExprTokenType::RPAREN) && !last_token_added_a_mul)
        {
          last_token_added_a_mul = true;
          return {ExprTokenType::OP, '*'};
        }
        else
        {
          ++pos;
          last_token_added_a_mul = false;
          return {ExprTokenType::LPAREN, '('};
        }
      }
      else if (ch == ')')
      {
        ++pos;
        return {ExprTokenType::RPAREN, ')'};
      }
  
      if (std::isdigit(ch))
      {
        std::string temp;
        do
        {
          temp += raw[pos];
          ++pos;
        } while (pos < raw.size() && (std::isdigit(raw[pos]) || raw[pos] == '.'));
        if (!parsing_negative)
          return {ExprTokenType::DIGIT, Frac<T>{Rational<T>{temp}}};
        else
        {
          parsing_negative = false;
          return {ExprTokenType::DIGIT, Frac<T>{Rational<T>{temp}}.negate()};
        }
      }
      else if (std::isalpha(ch) || ch == '{')
        // when parser adds '*' to "2{x}", ch might be '}'
      {
        std::string symbol;
        if (ch == '{')
        {
          ++pos;
          do
          {
            symbol += raw[pos];
            ++pos;
          } while (pos < raw.size() && raw[pos] != '}');
        }
        else
          symbol += ch;
        if (curr.type() == ExprTokenType::SYMBOL || curr.type() == ExprTokenType::DIGIT ||
            curr.type() == ExprTokenType::RPAREN)
        {
          last_token_added_a_mul = true;
          if (ch == '{')
            pos -= symbol.size() + 1;
          return {ExprTokenType::OP, '*'};
        }
        else
          ++pos;
        if (last_token_added_a_mul)
        {
          ++pos;
          last_token_added_a_mul = false;
        }
        if (!parsing_negative)
          return {ExprTokenType::SYMBOL, Frac<T>{{Term<T>{Real<T>{1}, symbol}}}};
        else
        {
          ++pos;
          parsing_negative = false;
          return {ExprTokenType::SYMBOL, Frac<T>{{Term<T>{Real<T>{-1}, symbol}}}};
        }
      }
      else
        throw Error("Unexpected '" + std::string(1, ch) + "'.");
      return curr;
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