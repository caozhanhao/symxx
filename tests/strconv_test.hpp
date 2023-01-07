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
#ifndef SYMXX_STRCONV_TEST_HPP
#define SYMXX_STRCONV_TEST_HPP

#include "unittest.hpp"

namespace symxx::test
{
  void strconv_test()
  {
    //ExprToken + xxx.to_string()
    SYMXX_MARK("STRCONV TEST");
    SYMXX_EXPECT_EQ("ExprParser 1", (ExprParser<int>("abcde").parse().to_string()), "a*b*c*d*e");
    SYMXX_EXPECT_EQ("ExprParser 2", (ExprParser<int>("{a}{b}{c}{d}{e}").parse().to_string()), "a*b*c*d*e");
    SYMXX_EXPECT_EQ("ExprParser 3", (ExprParser<int>("{a}{bi}{c}{d}{e}").parse().to_string()), "a*{bi}*c*d*e");
    SYMXX_EXPECT_EQ("ExprParser 4", (ExprParser<int>("{a}*{i}*{c}*{d}{e}").parse().to_string()), "a*i*c*d*e");
    SYMXX_EXPECT_EQ("ExprParser 5", (ExprParser<int>("a*{i}*{cc}*d{e}").parse().to_string()), "a*i*{cc}*d*e");
    SYMXX_EXPECT_EQ("ExprParser 6", (ExprParser<int>("x+1").parse().to_string()), "x+1");
    SYMXX_EXPECT_EQ("ExprParser 7", (ExprParser<int>("-1-x").parse().to_string()), "-1-x");
    SYMXX_EXPECT_EQ("ExprParser 8", (ExprParser<int>("-x-1").parse().to_string()), "-x-1");
    SYMXX_EXPECT_EQ("ExprParser 9", (ExprParser<int>("-x-x").parse().to_string()), "-x-x");
    SYMXX_EXPECT_EQ("ExprParser 10", (ExprParser<int>("-2x").parse().to_string()), "-2*x");
    SYMXX_EXPECT_EQ("ExprParser 11", (ExprParser<int>("3(1+2)").parse().to_string()), "3*(1+2)");
    SYMXX_EXPECT_EQ("ExprParser 12", (ExprParser<int>("4(2-4*6)").parse().to_string()), "4*(2-4*6)");
    SYMXX_EXPECT_EQ("ExprParser 13", (ExprParser<int>("(1+2)*7").parse().to_string()), "(1+2)*7");
    
    SYMXX_EXPECT_EQ("Frac 1", (ExprParser<int>("abcde").parse().try_eval()->to_string()), "abcde");
    SYMXX_EXPECT_EQ("Frac 2", (ExprParser<int>("{a}{b}{c}{d}{e}").parse().try_eval()->to_string()), "abcde");
    SYMXX_EXPECT_EQ("Frac 3", (ExprParser<int>("{a}{bi}{c}{d}{e}").parse().try_eval()->to_string()), "a{bi}cde");
    SYMXX_EXPECT_EQ("Frac 4", (ExprParser<int>("{a}*{i}*{c}*{d}{e}").parse().try_eval()->to_string()), "acdei");
    SYMXX_EXPECT_EQ("Frac 5", (ExprParser<int>("a*{i}*{cc}*d{e}").parse().try_eval()->to_string()), "a{cc}dei");
    SYMXX_EXPECT_EQ("Frac 6", (ExprParser<int>("x+1").parse().try_eval()->to_string()), "x+1");
    SYMXX_EXPECT_EQ("Frac 7", (ExprParser<int>("-1-x").parse().try_eval()->to_string()), "-x-1");
    SYMXX_EXPECT_EQ("Frac 8", (ExprParser<int>("-x-1").parse().try_eval()->to_string()), "-x-1");
    SYMXX_EXPECT_EQ("Frac 9", (ExprParser<int>("-x-x").parse().try_eval()->to_string()), "-2x");
    SYMXX_EXPECT_EQ("Frac 10", (ExprParser<int>("-2x").parse().try_eval()->to_string()), "-2x");
    SYMXX_EXPECT_EQ("Frac 11", (ExprParser<int>("3(1+2)").parse().try_eval()->to_string()), "9");
    SYMXX_EXPECT_EQ("Frac 12", (ExprParser<int>("4(2-4*6)").parse().try_eval()->to_string()), "-88");
    SYMXX_EXPECT_EQ("Frac 13", (ExprParser<int>("(1+2)*7").parse().try_eval()->to_string()), "21");
    
    SYMXX_EXPECT_EQ("Double & Rational 1", (ExprParser<int>("(1/2+2)/7").parse().try_eval()->to_string()), "5/14");
    SYMXX_EXPECT_EQ("Double & Rational 2", (ExprParser<int>("(0.5+2)/7").parse().try_eval()->to_string()), "5/14");
    SYMXX_EXPECT_EQ("Double & Rational 3", (ExprParser<int>("0.1+0.2").parse().try_eval()->to_string()), "3/10");
    SYMXX_EXPECT_EQ("Double & Rational 4", (ExprParser<int>("0.1*0.2").parse().try_eval()->to_string()), "1/50");
    
    SYMXX_EXPECT_EQ("Real 1", (nth_root<int>(2, 6).to_string()), "_/6");
    SYMXX_EXPECT_EQ("Real 2", (nth_root<int>(3, 6).to_string()), "_3_/6");
    SYMXX_EXPECT_EQ("Real 3", (nth_root<int>(9, Rational<int>{1, 3}).to_string()), "1/3_9_/6561");
    //3**9 == 19683, 19683 / 3 == 6561
  }
}
#endif
