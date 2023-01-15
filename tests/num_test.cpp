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
#include "unittest.hpp"

namespace symxx::test
{
  SYMXX_TEST(num)
  {
    //Rational
    Rational<int> s1{6, 2};
    Rational<int> s2{3, 1};
    Rational<int> s3{18, 6};
    Rational<int> s4{1, 3};
    SYMXX_EXPECT_EQ(s1, s2)
    SYMXX_EXPECT_EQ(s1, s3)
    SYMXX_EXPECT_EQ((Rational<int>{0, 100}), (Rational<int>{0, 50}));
    SYMXX_EXPECT_EQ(s1 + s2, (Rational<int>{6, 1}));
    SYMXX_EXPECT_EQ(s1 + s2 + s3 + s4, (Rational<int>{28, 3}));
    SYMXX_EXPECT_EQ(s1 - s2, (Rational<int>{0, 100}));
    SYMXX_EXPECT_EQ(s1 - s4, (Rational<int>{8, 3}));
    SYMXX_EXPECT_EQ(s1 * s2, (Rational<int>{9, 1}));
    SYMXX_EXPECT_EQ(s1 * s4, (Rational<int>{1, 1}));
    SYMXX_EXPECT_EQ(s1 / s2, (Rational<int>{1, 1}));
    SYMXX_EXPECT_EQ(s1 / s4, (Rational<int>{9, 1}));
    //Real
    Real<int> g2{1, 2, 2};//_/2
    Real<int> g3{1, 3, 2};//_/3
    Real<int> g4{1, 4, 2};//_/4==2
    Real<int> g4_4{1, 4, 4};//_4_/4==_/2
    //nth_root
    SYMXX_EXPECT_EQ(nth_root<int>(2, 6), (Real<int>{1, 6, 2}));
    SYMXX_EXPECT_EQ(nth_root<int>(2, 8), g2 * 2);
    SYMXX_EXPECT_EQ(nth_root<int>(2, 9), 3);
    SYMXX_EXPECT_EQ(nth_root<int>(2, 36), 6);
    SYMXX_EXPECT_EQ(nth_root<int>(4, 4), g2);
    SYMXX_EXPECT_EQ(nth_root<int>(2, {1, 3}), (Real<int>({1, 3}, 3, 2)));
    SYMXX_EXPECT_EQ(nth_root<int>(2, {1, 9}), (Rational<int>(1, 3)));
    SYMXX_EXPECT_EQ(nth_root<__int128>(4, 788860905221011700), nth_root<__int128>(4, 788860905221011700));
    SYMXX_EXPECT_EQ(nth_root<__int128>(2, 788860905221011700), nth_root<__int128>(2, 7888609052210117) * 10);
    SYMXX_EXPECT_EQ(nth_root<__int128>(8, 63527879748485376), 126);
    SYMXX_EXPECT_EQ(g4, 2);
    SYMXX_EXPECT_TRUE(g2 > (Rational<int>{141, 100}));
    SYMXX_EXPECT_TRUE(g4 > g3);
    SYMXX_EXPECT_TRUE(g3 > g2);
    SYMXX_EXPECT_TRUE(g4 > g2);
    SYMXX_EXPECT_TRUE(g4_4 == g2);
    SYMXX_EXPECT_TRUE(g4_4 < g3);
    SYMXX_EXPECT_TRUE(g4_4 < g4);
    SYMXX_EXPECT_TRUE(-1 < 0);
    SYMXX_EXPECT_TRUE(1 > 0);
    SYMXX_EXPECT_TRUE(g2 > 0);
    SYMXX_EXPECT_TRUE(g2.negate() < 0);
    SYMXX_EXPECT_TRUE(g2.negate() > g3.negate());
    SYMXX_EXPECT_TRUE((Real<int>{-4} < g3.negate()));
    SYMXX_EXPECT_EQ(g4 + 2, 4);
    SYMXX_EXPECT_EQ(g4 - 2, 0);
    SYMXX_EXPECT_EQ(g4 * 2, 4);
    SYMXX_EXPECT_EQ(g2 * g3, nth_root<int>(2, 6));
    SYMXX_EXPECT_EQ(g2 * g3, (Real<int>{1, 6, 2}));
    SYMXX_EXPECT_EQ(g2 * g4, (Real<int>{1, 8, 2}));
    SYMXX_EXPECT_EQ(g2 * g4_4, 2);
    SYMXX_EXPECT_EQ(g2 * g2, 2);
    SYMXX_EXPECT_EQ(g2 * g2.inverse(), 1);
    SYMXX_EXPECT_EQ((g3 * Real<int>{1, Rational<int>{1, 3}, 2}), 1);
    SYMXX_EXPECT_EQ(g2 * 2, nth_root<int>(2, 8));
    SYMXX_EXPECT_EQ(g4 / g2, g2);
    SYMXX_EXPECT_EQ(g4 / g4_4, g2);
    SYMXX_EXPECT_EQ((Real<int>{1, 6, 2}) / g3, g2);
    SYMXX_EXPECT_EQ(g2 / g3, (Real<int>{1, {2, 3}, 2}));
    SYMXX_EXPECT_EQ(g3 / g2, (Real<int>{1, {3, 2}, 2}));
    SYMXX_EXPECT_EQ(g3 / g3, 1);
  }
}
