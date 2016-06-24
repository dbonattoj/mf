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

#ifndef MF_EXCEPTIONS_H_
#define MF_EXCEPTIONS_H_

#include <stdexcept>
#include <string>

#define MF_DEFINE_EXCEPTION_(name, base) \
	class name : public base { \
	public: \
		explicit name(const std::string& what) : base(what) { } \
		explicit name(const char* what) : base(what) { } \
	};


namespace mf {
	
MF_DEFINE_EXCEPTION_(invalid_flow_graph, std::logic_error);

MF_DEFINE_EXCEPTION_(failed_assertion, std::runtime_error);

MF_DEFINE_EXCEPTION_(sequencing_error, std::runtime_error);
MF_DEFINE_EXCEPTION_(ply_importer_error, std::runtime_error);

}

#endif
