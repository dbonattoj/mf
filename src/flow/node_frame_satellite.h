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

#ifndef MF_FLOW_NODE_FRAME_SATELLITE_H_
#define MF_FLOW_NODE_FRAME_SATELLITE_H_

#include <iostream>
#include <new>

namespace mf { namespace flow {

/// Satellite object attached to frame.
/** Each frame being passed between nodes contains an instance of this class. */
class node_frame_satellite {
private:
	int stuff[100];
	
public:	
	static void* operator new (std::size_t sz) {
		std::cout << "CONS" << std::endl;
		return ::operator new(sz);
	}
	
	static void operator delete (void* ptr) {
		std::cout << "DEST" << std::endl;
		::operator delete(ptr);
	}
};

inline bool operator==(node_frame_satellite, node_frame_satellite) { return true; }

}}

#endif
