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
#include <string>
namespace symxx
{
  class ExprParser
  {
    ExprParser(const std::string &str)
    {
      for (auto it = str.cbegin(); it < str.cend(); ++it)
      {
      }
    }
  };
}
#endif