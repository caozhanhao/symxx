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
#ifndef SYMXX_PARSER_HPP
#define SYMXX_PARSER_HPP
#include "num.hpp"
#include "expr.hpp"
#include "error.hpp"
#include <string>
#include <stack>
#include <variant>
#include <map>
namespace symxx
{
  enum class TokenType
  {
    OP,
    LPAREN,
    RPAREN,
    DIGIT,
    SYMBOL
  };
  template <typename T>
  class Token
  {
  private:
    std::variant<char, Frac<T>> val;
    TokenType token_type;

  public:
    template <typename U>
    Token(TokenType t, U &&u)
        : val(std::forward<U>(u)), token_type(t) {}

    auto type() const { return token_type; }
    auto frac() const { return std::get<Frac<T>>(val); }
    auto ch() const { return std::get<char>(val); }
  };
  template <typename T>
  class Parser
  {
  private:
    std::string raw;
    std::size_t pos;
    Token<T> curr;
    bool parsing_end;
    bool added_mul;
    bool parsing_negative;

  public:
    Parser(const std::string &r)
        : raw(r), pos(0), curr(TokenType::OP, '\0'), parsing_end(false), added_mul(false), parsing_negative(false) {}

    Frac<T> parse()
    {
      pos = 0;
      std::stack<Frac<T>> num;
      std::stack<char> op;
      auto handle = [&num, &op]()
      {
        if (num.size() < 2 || op.empty())
          throw Error(SYMXX_ERROR_LOCATION, __func__, "Invaild string.");
        auto lhs = num.top();
        num.pop();
        auto rhs = num.top();
        num.pop();
        switch (op.top())
        {
        case '+':
          num.push(rhs + lhs);
          break;
        case '-':
          num.push(rhs - lhs);
          break;
        case '*':
          num.push(rhs * lhs);
          break;
        case '/':
          num.push(rhs / lhs);
          break;
        case '^':
          num.push(rhs ^ (lhs.eval().template to<Rational<T>>()));
          break;
        default:
          throw Error(SYMXX_ERROR_LOCATION, __func__, "Unexpected token.");
          break;
        }
        op.pop();
      };
      while (true)
      {
        next();
        if (parsing_end)
          break;

        if (curr.type() == TokenType::OP)
        {
          while (!op.empty() && great(op.top(), curr.ch()))
            handle();
          op.push(curr.ch());
          continue;
        }
        else if (curr.type() == TokenType::LPAREN)
        {
          op.push('(');
          continue;
        }
        else if (curr.type() == TokenType::RPAREN)
        {
          while (!num.empty() && op.top() != '(')
            handle();
          op.pop();
          continue;
        }
        else if (curr.type() == TokenType::DIGIT || curr.type() == TokenType::SYMBOL)
        {
          num.push(curr.frac());
          continue;
        }
        else
          throw Error(SYMXX_ERROR_LOCATION, __func__, "Unexpected token.");
      }
      while (!op.empty())
        handle();
      if (num.size() != 1)
        throw Error(SYMXX_ERROR_LOCATION, __func__, "Invaild string.");
      return num.top();
    }

  private:
    void next()
    {
      curr = get_token();
    }
    Token<T> get_token()
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
          return {TokenType::OP, '^'};
        }
        else if (ch == '-' && (curr.type() == TokenType::LPAREN || (curr.type() == TokenType::OP && curr.ch() == '\0')))
        {
          parsing_negative = true;
          pos++;
          ch = raw[pos];
        }
        else
        {
          ++pos;
          return {TokenType::OP, ch};
        }
      }
      else if (ch == '(')
      {
        if ((curr.type() == TokenType::SYMBOL || curr.type() == TokenType::DIGIT || curr.type() == TokenType::RPAREN) && !added_mul)
        {
          added_mul = true;
          return {TokenType::OP, '*'};
        }
        else
        {
          ++pos;
          added_mul = false;
          return {TokenType::LPAREN, '('};
        }
      }
      else if (ch == ')')
      {
        ++pos;
        return {TokenType::RPAREN, ')'};
      }

      std::string temp;
      if (std::isdigit(ch))
      {
        do
        {
          temp += raw[pos];
          ++pos;
        } while (pos < raw.size() && std::isdigit(raw[pos]));
        if (!parsing_negative)
          return {TokenType::DIGIT, Frac<T>{Rational<T>{temp}}};
        else
        {
          parsing_negative = false;
          return {TokenType::DIGIT, Frac<T>{Rational<T>{temp}}.opposite()};
        }
      }
      else if (std::isalpha(ch))
      {
        if ((curr.type() == TokenType::SYMBOL || curr.type() == TokenType::DIGIT) && !added_mul)
        {
          added_mul = true;
          return {TokenType::OP, '*'};
        }
        else
        {
          ++pos;
          added_mul = false;
          if (!parsing_negative)
            return {TokenType::SYMBOL, Frac<T>{{{Term<T>{1, std::string(1, ch)}}}}};
          else
          {
            parsing_negative = false;
            return {TokenType::SYMBOL, Frac<T>{{{Term<T>{-1, std::string(1, ch)}}}}};
          }
        }
      }
      else
        throw Error(SYMXX_ERROR_LOCATION, __func__, "Unexpected '" + std::string(1, ch) + "'.");
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
          {'^', 1000}};
      ;
      return priority.at(a) > priority.at(b);
    }
  };
}
#endif