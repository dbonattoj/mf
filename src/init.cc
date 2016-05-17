#include "common.h"
#include "debug.h"
#include "eigen.h"
#include "os/os.h"

namespace mf {

namespace {
	///< Initialization function.
	/** Needs to be called when library is loaded, before any other functions. */
	void initialize_mf_() {
		initialize_debug();
		Eigen::initParallel();
	}
}

#if defined(MF_COMPILER_CLANG) || defined(MF_COMPILER_GCC)
static void init() __attribute__((constructor));

void init() {
	initialize_mf_();
}
#endif


}
