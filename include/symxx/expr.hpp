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
#ifndef SYMXX_EXPR_HPP
#define SYMXX_EXPR_HPP
#include "error.hpp"
#include "num.hpp"
#include "frac.hpp"
#include <iostream>
#include <variant>
namespace symxx
{
  enum class NodeType
  {
    OP,
    FRAC
  };
  
  template<typename T>
  class ExprNode;
  
  template<typename T>
  struct OpData
  {
    char op;
    ExprNode<T> *lhs;
    ExprNode<T> *rhs;
    
    OpData() : op('\0') {}
    
    OpData(char o, ExprNode<T> *l, ExprNode<T> *r)
        : op(o), lhs(l), rhs(r) {}
    
    OpData(const OpData &od)
    {
      op = od.op;
      if (od.lhs != nullptr)
        lhs = new ExprNode<T>(*od.lhs);
      if (od.rhs != nullptr)
        rhs = new ExprNode<T>(*od.rhs);
    }
  
    ~OpData()
    {
      if (lhs != nullptr)
      {
        delete lhs;
      }
      if (rhs != nullptr)
      {
        delete rhs;
      }
    }
  };
  
  template<typename T>
  class ExprNode
  {
  private:
    std::variant<Frac<T>, OpData<T>> val;
    NodeType node_type;

  public:
    ExprNode()
        : node_type(NodeType::FRAC), val(Frac<T>{0}) {}
  
    ExprNode(char op, ExprNode *lhs, ExprNode *rhs)
        : val(OpData{op, lhs, rhs}), node_type(NodeType::OP) {}
  
    ExprNode(const Frac<T> &frac) : val(frac), node_type(NodeType::FRAC) {}
  
    ExprNode(const ExprNode &nd) : val(nd.val), node_type(nd.node_type) {}
  
    ExprNode &substitute(Environment<T> e)
    {
      if (node_type == NodeType::FRAC)
      {
        frac().substitute(e);
        return *this;
      }
      auto &data = opdata();
      data.lhs->substitute(e);
      data.rhs->substitute(e);
      normalize();
      return *this;
    }
    
    std::unique_ptr<Frac<T>> try_eval() const
    {
      if (node_type == NodeType::FRAC)
        return std::make_unique<Frac<T>>(frac());
      auto &data = opdata();
      auto lhsv = data.lhs->try_eval();
      auto rhsv = data.rhs->try_eval();
      if (lhsv == nullptr || rhsv == nullptr)
        return nullptr;
      switch (data.op)
      {
        case '+':
          return std::make_unique<Frac<T>>(*lhsv + *rhsv);
        case '-':
          return std::make_unique<Frac<T>>(*lhsv - *rhsv);
        case '*':
          return std::make_unique<Frac<T>>(*lhsv * *rhsv);
        case '/':
          return std::make_unique<Frac<T>>(*lhsv / *rhsv);
        case '^':
        {
          auto power = rhsv->try_eval();
          if (power == nullptr || !power->is_rational())
          {
            return nullptr;
          }
          return std::make_unique<Frac<T>>(lhsv->pow(rhsv->template to<Rational<T>>()));
        }
          break;
        default:
          symxx_unreachable();
          break;
      }
      return nullptr;
    }
  
    std::unique_ptr<long double> try_eval(const std::map<std::string, long double> &v) const
    {
      if (node_type == NodeType::FRAC)
      {
        return frac().try_eval(v);
      }
      auto &data = opdata();
      auto lhsv = data.lhs->try_eval(v);
      auto rhsv = data.rhs->try_eval(v);
      if (auto lhsv = data.lhs->try_eval(v), rhsv = data.rhs->try_eval(v); lhsv == nullptr || rhsv == nullptr)
      {
        return nullptr;
      }
      switch (data.op)
      {
        case '+':
          return std::make_unique<long double>(*lhsv + *rhsv);
        case '-':
          return std::make_unique<long double>(*lhsv - *rhsv);
        case '*':
          return std::make_unique<long double>(*lhsv * *rhsv);
        case '/':
          return std::make_unique<long double>(*lhsv / *rhsv);
        case '^':
          return std::make_unique<long double>(std::pow(*lhsv, *rhsv));
          break;
        default:
          symxx_unreachable();
          break;
      }
      symxx_unreachable();
      return nullptr;
    }
  
    ExprNode &normalize()
    {
      if (node_type == NodeType::FRAC)
      {
        return *this;
      }
      auto &data = opdata();
      auto lhsv = data.lhs->try_eval();
      auto rhsv = data.rhs->try_eval();
      if (lhsv == nullptr || rhsv == nullptr)
      {
        data.rhs->normalize();
        data.lhs->normalize();
        return *this;
      }
      switch (data.op)
      {
        case '+':
          val = {*lhsv + *rhsv};
          break;
        case '-':
          val = {*lhsv - *rhsv};
          break;
        case '*':
          val = {*lhsv * *rhsv};
          break;
        case '/':
          val = {*lhsv / *rhsv};
          break;
        case '^':
        {
          auto power = rhsv->try_eval();
          if (power == nullptr || !power->is_rational())
          {
            data.lhs->normalize();
            data.rhs->normalize();
            return *this;
          }
          val = lhsv->pow(power->template to<Rational<T>>());
        }
          break;
        default:
          symxx_unreachable();
          break;
      }
      node_type = NodeType::FRAC;
      return *this;
    }
  
    std::string to_string() const
    {
      if (type() == NodeType::FRAC)
        return frac().to_string();
    
      std::string ret;
      auto &opd = opdata();
      if (withparen(true))
      {
        ret += "(" + opd.lhs->to_string() + ")";
      }
      else
      {
        ret += opd.lhs->to_string();
      }
      ret += opd.op;
      if (withparen(false))
      {
        ret += "(" + opd.rhs->to_string() + ")";
      }
      else
      {
        ret += opd.rhs->to_string();
      }
      return ret;
    }
  
    std::string to_tex() const
    {
      if (type() == NodeType::FRAC)
      {
        return frac().to_tex();
      }
    
      std::string ret;
      auto &opd = opdata();
    
      if (opd.op == '/')
      {
        ret += "\\frac{";
      }
      if (withparen(true))
      {
        ret += "(" + opd.lhs->to_tex() + ")";
      }
      else
      {
        ret += opd.lhs->to_tex();
      }
    
      if (opd.op == '/')
      {
        ret += "}{";
      }
      else if (opd.op == '+' || opd.op == '-')
      {
        ret += opd.op;
      }
      else if (opd.op == '*')
      {
        ret += "\\times ";
      }
      else if (opd.op == '^')
      {
        ret += "^{";
      }
    
      if (withparen(false))
      {
        ret += "(" + opd.rhs->to_tex() + ")";
      }
      else
      {
        ret += opd.rhs->to_tex();
      }
    
      if (opd.op == '/' || opd.op == '^')
      {
        ret += "}";
      }
      return ret;
    }

  private:
    bool withparen(bool left) const
    {
      if (type() == NodeType::FRAC)
      {
        if (frac().output_need_paren())
        {
          return true;
        }
        return false;
      }
  
      auto &opd = opdata();
      auto op = opd.op;
  
      if (left)
      {
        if (opd.lhs->type() != NodeType::OP)
        {
          if (opd.lhs->frac().output_need_paren())
          {
            return true;
          }
          return false;
        }
        char leftop = opd.lhs->opdata().op;
        switch (op)
        {
          case '*':
          case '/':
            if (leftop == '+' || leftop == '-')
              return true;
            break;
          case '^':
            return true;
            break;
        }
      }
      else
      {
        if (opd.rhs->type() != NodeType::OP)
        {
          if (opd.rhs->frac().output_need_paren())
          {
            return true;
          }
          return false;
        }
        char rightop = opd.rhs->opdata().op;
        switch (op)
        {
          case '*':
            if (rightop == '+' || rightop == '-')
              return true;
            break;
          case '/':
            if (rightop == '+' || rightop == '-' || rightop == '*' || rightop == '/')
              return true;
            break;
          case '-':
            if (rightop == '+' || rightop == '-')
              return true;
            break;
          case '^':
            return true;
            break;
        }
      }
      return false;
    }
  
    auto type() const { return node_type; }
  
    auto &frac() { return std::get<Frac<T>>(val); }
  
    auto &opdata() { return std::get<OpData<T>>(val); }
  
    auto &frac() const { return std::get<Frac<T>>(val); }
  
    auto &opdata() const { return std::get<OpData<T>>(val); }
  
  };
  
  template<typename U>
  std::ostream &operator<<(std::ostream &os, const ExprNode<U> &i)
  {
    os << i.to_string();
    return os;
  }
}
#endif