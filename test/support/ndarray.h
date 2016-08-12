/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef MF_TESTSUPPORT_NDARRAY_H_
#define MF_TESTSUPPORT_NDARRAY_H_

#include <mf/ndarray/ndarray.h>
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
	
	std::cerr << "sequence mismatch: " << std::endl << str.str() << std::endl;	
	
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
	
	std::cerr << "sequence mismatch (backwards): " << std::endl << str.str() << std::endl;	
	
	return false;
}

ndarray<2, int> make_frame(const ndsize<2>& shape, int i);

int frame_index(const ndarray_view<2, int>&, bool verify = false);

bool compare_frames(const ndsize<2>& shape, const ndarray_view<3, int>& frames, const std::vector<int>& is);

}}

#endif
