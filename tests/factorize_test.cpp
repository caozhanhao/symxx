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
  SYMXX_TEST(factorize)
  {
    using symxx::factorize;
    using symxx::factorize_internal::is_prime;
    SYMXX_EXPECT_TRUE(is_prime<int64_t>(2));
    SYMXX_EXPECT_TRUE(is_prime<int64_t>(3));
    SYMXX_EXPECT_TRUE(is_prime<int64_t>(5));
    SYMXX_EXPECT_TRUE(is_prime<int64_t>(7));
    SYMXX_EXPECT_TRUE(is_prime<int64_t>(83));
    SYMXX_EXPECT_TRUE(is_prime<int64_t>(271));
    SYMXX_EXPECT_TRUE(is_prime<int64_t>(48541));
    SYMXX_EXPECT_TRUE(is_prime<int64_t>(47119));
    SYMXX_EXPECT_TRUE(is_prime<int64_t>(28351));
    SYMXX_EXPECT_TRUE(is_prime<int64_t>(100001611));
    SYMXX_EXPECT_TRUE(is_prime<__int128_t>(10000000000000069));
    SYMXX_EXPECT_TRUE(is_prime<__int128_t>(1000000000000001323));
    SYMXX_EXPECT_TRUE(is_prime<__int128_t>(1000000000000002493));
    SYMXX_EXPECT_FALSE(is_prime<int64_t>(15));
    SYMXX_EXPECT_FALSE(is_prime<int64_t>(77));
    SYMXX_EXPECT_FALSE(is_prime<int64_t>(4555551));
    
    std::multiset<__int128_t> s;
    factorize<__int128_t>(12345, s);
    SYMXX_EXPECT_EQ(to_str(s), to_str(std::multiset<__int128_t>{3, 5, 823}));
    s.clear();
    factorize<__int128_t>(1234554321, s);
    SYMXX_EXPECT_EQ(to_str(s), to_str(std::multiset<__int128_t>{3, 7, 11, 13, 37, 41, 271}));
    s.clear();
    factorize<__int128_t>(6352787974848537642, s);
    SYMXX_EXPECT_EQ(to_str(s), to_str(std::multiset<__int128_t>{2, 3, 7, 257, 1189003, 494992931}));
    s.clear();
  }
}
