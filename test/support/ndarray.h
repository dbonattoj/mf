#ifndef MF_TESTSUPPORT_NDARRAY_H_
#define MF_TESTSUPPORT_NDARRAY_H_

#include "../../src/ndarray/ndarray.h"
#include <iostream>
#include <sstream>
#include <vector>

namespace mf { namespace test {

template<typename Buffer>
bool compare_sequence_forwards_(const Buffer& buf, const std::vector<int>& seq) {
	std::ostringstream str;

	auto vec_it = seq.begin();
	auto it = buf.begin();
	for(; (it != buf.end()) && (vec_it != seq.end()); ++it, ++vec_it) {
		str << std::hex << "got:" << *it << "  wanted:" << *vec_it << std::endl;
		if(*vec_it != *it) break;
	}
	if( (it == buf.end()) && (vec_it == seq.end()) ) return true;
	else if( (it == buf.end()) && (vec_it != seq.end()) ) str << "ended early" << std::endl;
	else if( (it != buf.end()) && (vec_it == seq.end()) ) str << "ended late" << std::endl;
	
	std::cout << "sequence mismatch: " << std::endl << str.str() << std::endl;	
	
	return false;
}


template<typename Buffer>
bool compare_sequence_(const Buffer& buf, const std::vector<int>& seq) {
	if(! compare_sequence_forwards_(buf, seq)) return false;
	
	std::ostringstream str;

	auto vec_it = seq.end();
	auto it = buf.end();
	
	do {
		--it; --vec_it;
		str << std::hex << "got:" << *it << "  wanted:" << *vec_it << std::endl;
		if(*vec_it != *it) break;
	} while( (it != buf.begin()) && (vec_it != seq.begin()) );
	if( (it == buf.begin()) && (vec_it == seq.begin()) ) return true;
	else if( (it == buf.begin()) && (vec_it != seq.begin()) ) str << "arrived back early" << std::endl;
	else if( (it != buf.begin()) && (vec_it == seq.begin()) ) str << "arrived back late" << std::endl;
	
	std::cout << "sequence mismatch (backwards): " << std::endl << str.str() << std::endl;	
	
	return false;
}

ndarray<2, int> make_frame(const ndsize<2>& shape, int i);

int frame_index(const ndarray_view<2, int>&);

bool compare_frames(const ndsize<2>& shape, const ndarray_view<3, int>& frames, const std::vector<int>& is);

}}

#endif
