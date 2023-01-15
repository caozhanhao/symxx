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

#define SYMXX_ENABLE_INT128
//#define SYMXX_ENABLE_HUGE
#include <string_view>

constexpr std::string_view SYMXX_VERSION = "0.0.1";

#include "symxx/symxx.hpp"

int main()
{
  symxx::CLI s;
  s.mainloop();
  return 0;
}