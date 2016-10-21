#ifndef MF_TEST_FLOW_ACTIVATION_H_
#define MF_TEST_FLOW_ACTIVATION_H_

#include "../../src/common.h"
#include <valarray>

namespace mf { namespace test {

using activation_sequence_type = std::valarray<bool>;

inline activation_sequence_type random_activation_sequence(std::size_t count) {
	activation_sequence_type act(count);
	for(std::ptrdiff_t i = 0; i < count; ++i) act[i] = (randint(0, 2) == 0);
	return act;
}

inline std::vector<int> expected_frame_index_sequence(const activation_sequence_type& act) {
	std::vector<int> seq(act.size());
	for(std::ptrdiff_t i = 0; i < seq.size(); ++i) {
		if(act[i]) seq[i] = i;
		else seq[i] = -2;
	}
	return seq;
}

}}

#endif
