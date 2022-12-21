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
  template <typename T>
  class ExprNode;
  template <typename T>
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
        delete lhs;
      if (rhs != nullptr)
        delete rhs;
    }
  };
  template <typename T>
  class ExprNode
  {
    template <typename U>
    friend std::ostream &operator<<(std::ostream &os, const ExprNode<U> &i);
    template <typename U>
    friend bool withparen(const ExprNode<U> &node, bool left);

  private:
    std::variant<Frac<T>, OpData<T>> val;
    NodeType node_type;

  public:
    ExprNode(char op, ExprNode *lhs, ExprNode *rhs)
        : val(OpData{op, lhs, rhs}), node_type(NodeType::OP) {}
    ExprNode(const Frac<T> &frac) : val(frac), node_type(NodeType::FRAC) {}
    ExprNode(const ExprNode &nd) : val(nd.val), node_type(nd.node_type) {}
    ExprNode set_var(const std::map<std::string, Real<T>> &e) const
    {
      auto a = *this;
      Environment<T> env = std::make_shared<std::map<std::string, Real<T>>>(e);
      a.set_env(env);
      return a;
    }
    void set_env(Environment<T> e)
    {
      if (node_type == NodeType::FRAC)
      {
        frac().set_env(e);
        return;
      }
      auto &data = opdata();
      data.lhs->set_env(e);
      data.rhs->set_env(e);
      reduce();
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
          return nullptr;
        return std::make_unique<Frac<T>>(*lhsv ^ rhsv->template to<Rational<T>>());
      }
      break;
      default:
        throw Error(SYMXX_ERROR_LOCATION, __func__, "Unexpected op '" + std::string(1, data.op) + "'.");
        break;
      }
      return nullptr;
    }
    void reduce()
    {
      if (node_type == NodeType::FRAC)
        return;
      auto &data = opdata();
      auto lhsv = data.lhs->try_eval();
      auto rhsv = data.rhs->try_eval();
      if (lhsv == nullptr)
        data.lhs->reduce();
      if (rhsv == nullptr)
        data.rhs->reduce();
      if (lhsv == nullptr || rhsv == nullptr)
        return;
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
          data.lhs->reduce();
          return;
        }
        val = *lhsv ^ power->template to<Rational<T>>();
      }
      break;
      default:
        throw Error(SYMXX_ERROR_LOCATION, __func__, "Unexpected op '" + std::string(1, data.op) + "'.");
        break;
      }
      node_type = NodeType::FRAC;
    }

  private:
    auto type() const { return node_type; }
    auto &frac() { return std::get<Frac<T>>(val); }
    auto &opdata() { return std::get<OpData<T>>(val); }
    auto &frac() const { return std::get<Frac<T>>(val); }
    auto &opdata() const { return std::get<OpData<T>>(val); }
  };

  template <typename U>
  bool withparen(const ExprNode<U> &node, bool left)
  {
    if (node.type() == NodeType::FRAC)
      return false;
    auto &opdata = node.opdata();
    auto op = opdata.op;
    if (opdata.lhs == nullptr || opdata.rhs == nullptr)
      return false;

    if (left)
    {
      if (opdata.lhs->type() != NodeType::OP)
        return false;
      char leftop = opdata.lhs->opdata().op;
      switch (op)
      {
      case '*':
      case '/':
        if (leftop == '+' || leftop == '-')
          return true;
      case '^':
        return true;
      }
    }
    else
    {
      if (opdata.rhs->type() != NodeType::OP)
        return false;
      char rightop = opdata.rhs->opdata().op;
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

  template <typename U>
  std::ostream &operator<<(std::ostream &os, const ExprNode<U> &i)
  {
    if (i.type() == NodeType::FRAC)
    {
      os << i.frac();
      return os;
    }

    auto &opdata = i.opdata();
    if (withparen<U>(i, true))
      os << "(" << *opdata.lhs << ")";
    else
      os << *opdata.lhs;
    os << opdata.op;
    if (withparen(i, false))
      os << "(" << *opdata.rhs << ")";
    else
      os << *opdata.rhs;
    return os;
  }
}
#endif