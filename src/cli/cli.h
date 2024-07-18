#pragma once
#include <docopt.h>
#include <map>
#include <string>

using CliArgs = std::map<std::string, docopt::value>;

auto cli(int argc, char** argv, std::string version) -> CliArgs;
