// Copyright (c) 2015-2016 The Khronos Group Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#include "spirv-tools/libspirv.h"
#include "tools/cfg/bin_to_dot.h"
#include "tools/io.h"
#include "tools/util/flags.h"

static const auto kDefaultEnvironment = SPV_ENV_UNIVERSAL_1_6;
static const std::string kHelpText =
    R"(%s - Show the control flow graph in GraphiViz "dot" form. EXPERIMENTAL

Usage: %s [options] [<filename>]

The SPIR-V binary is read from <filename>. If no file is specified,
or if the filename is "-", then the binary is read from standard input.

Options:

  -h, --help      Print this help.
  --version       Display version information.

  -o <filename>   Set the output filename.
                  Output goes to standard output if this option is
                  not specified, or if the filename is "-".
)";

// clang-format off
FLAG_SHORT_bool(  h,       /* default_value= */ false, /* required= */ false);
FLAG_LONG_bool(   help,    /* default_value= */ false, /* required= */false);
FLAG_LONG_bool(   version, /* default_value= */ false, /* required= */ false);
FLAG_SHORT_string(o,       /* default_value= */ "",    /* required= */ false);
// clang-format on

int main(int, const char** argv) {
  if (!flags::Parse(argv)) {
    return 1;
  }

  if (flags::h.value() || flags::help.value()) {
    printf(kHelpText.c_str(), argv[0], argv[0]);
    return 0;
  }

  if (flags::version.value()) {
    printf("%s EXPERIMENTAL\n", spvSoftwareVersionDetailsString());
    printf("Target: %s\n", spvTargetEnvDescription(kDefaultEnvironment));
    return 0;
  }

  if (flags::positional_arguments.size() != 1) {
    fprintf(stderr, "error: exactly one input file must be specified.\n");
    return 1;
  }

  std::string inFile = flags::positional_arguments[0];
  std::string outFile = flags::o.value();

  // Read the input binary.
  std::vector<uint32_t> contents;
  if (!ReadBinaryFile(inFile.c_str(), &contents)) return 1;
  spv_context context = spvContextCreate(kDefaultEnvironment);
  spv_diagnostic diagnostic = nullptr;

  std::stringstream ss;
  auto error =
      BinaryToDot(context, contents.data(), contents.size(), &ss, &diagnostic);
  if (error) {
    spvDiagnosticPrint(diagnostic);
    spvDiagnosticDestroy(diagnostic);
    spvContextDestroy(context);
    return error;
  }
  std::string str = ss.str();
  WriteFile(outFile.empty() ? nullptr : outFile.c_str(), "w", str.data(),
            str.size());

  spvDiagnosticDestroy(diagnostic);
  spvContextDestroy(context);

  return 0;
}
