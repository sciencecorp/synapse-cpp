#include "cli.h"

static const char USAGE[] =
R"(Synapse C++ Client CLI

  Usage:
    synapse discover [--auth_code=<code>] [--timeout=<ms>]
    synapse info <uri>
    synapse configure <uri> [--config=<filepath>]
    synapse start <uri>
    synapse stop <uri>
    synapse stream read <uri> [--multicast=<group>]
    synapse stream write <uri>
    synapse (-h | --help)
    synapse --version 

  Options:
    -h --help               Show this screen.
    --version               Show version.
    -c --config=<filepath>  Path to configuration file.
    --auth_code=<code>      Authorization code for device discovery [default: ].
    --multicast=<group>     Multicast group for streaming [default: ].
    --timeout=<ms>          Timeout in milliseconds for device discovery [default: 2000].
)";
static const bool show_help = true;

auto cli(int argc, char** argv, std::string version) -> CliArgs {
  return docopt::docopt(
    USAGE,
    { argv + 1, argv + argc },
    show_help,
    version
  );
}
