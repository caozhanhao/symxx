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
#ifndef SYMXX_DTOA_TEST_HPP
#define SYMXX_DTOA_TEST_HPP

#include "unittest.hpp"

namespace symxx::test
{
  void dtoa_test()
  {
    SYMXX_MARK("DTOA TEST");
    SYMXX_EXPECT_EQ("Dtoa 1", ::symxx::dtoa(0.0), "0.0");
    SYMXX_EXPECT_EQ("Dtoa 2", ::symxx::dtoa(-0.0), "-0.0");
    SYMXX_EXPECT_EQ("Dtoa 3", ::symxx::dtoa(1.0), "1.0");
    SYMXX_EXPECT_EQ("Dtoa 4", ::symxx::dtoa(-1.0), "-1.0");
    SYMXX_EXPECT_EQ("Dtoa 5", ::symxx::dtoa(1.2345), "1.2345");
    SYMXX_EXPECT_EQ("Dtoa 6", ::symxx::dtoa(1.2345678), "1.2345678");
    SYMXX_EXPECT_EQ("Dtoa 7", ::symxx::dtoa(0.123456789012), "0.123456789012");
    SYMXX_EXPECT_EQ("Dtoa 8", ::symxx::dtoa(1234567.8), "1234567.8");
    SYMXX_EXPECT_EQ("Dtoa 9", ::symxx::dtoa(-79.39773355813419), "-79.39773355813419");
    SYMXX_EXPECT_EQ("Dtoa 10", ::symxx::dtoa(-36.973846435546875), "-36.973846435546875");
    SYMXX_EXPECT_EQ("Dtoa 11", ::symxx::dtoa(0.000001), "0.000001");
    SYMXX_EXPECT_EQ("Dtoa 12", ::symxx::dtoa(0.0000001), "1e-7");
    SYMXX_EXPECT_EQ("Dtoa 13", ::symxx::dtoa(1e30), "1e30");
    SYMXX_EXPECT_EQ("Dtoa 14", ::symxx::dtoa(1.234567890123456e30), "1.234567890123456e30");
    SYMXX_EXPECT_EQ("Dtoa 15", ::symxx::dtoa(5e-324), "5e-324");
    SYMXX_EXPECT_EQ("Dtoa 16", ::symxx::dtoa(2.225073858507201e-308), "2.225073858507201e-308");
    SYMXX_EXPECT_EQ("Dtoa 17", ::symxx::dtoa(2.2250738585072014e-308), "2.2250738585072014e-308");
    SYMXX_EXPECT_EQ("Dtoa 18", ::symxx::dtoa(1.7976931348623157e308), "1.7976931348623157e308");
  }
}
#endif
