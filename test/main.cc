#define CATCH_CONFIG_RUNNER
#include <catch.hpp>
#include "../src/debug.h"

using namespace mf;

int main(int argc, const char* argv[]) {
	set_debug_mode(debug_mode::cerr);
	set_debug_filter({"node"});

	return Catch::Session().run(argc, argv);
}
