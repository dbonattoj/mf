#include "common.h"
#include "eigen.h"
#include "utility/os.h"

namespace mf {

namespace {
	///< Initialization function.
	/** Needs to be called when library is loaded, before any other functions. */
	void initialize_mf_() {
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
