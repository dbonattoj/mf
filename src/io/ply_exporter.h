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

#include <fstream>
#include <string>
#include "../point_cloud/point.h"
#include "../utility/io.h"
#include "../nd/ndarray_view.h"

namespace mf {

/// Exports point cloud into PLY file.
/** Can generate binary or ASCII format, and either only XYZ coordinates or with color, weight and normals. */
class ply_exporter {
private:
	std::ofstream file_;
	std::ofstream::pos_type vertex_count_string_position_;
	std::size_t count_ = 0;
	
	const line_delimitor line_delimitor_;
	bool full_;
	bool ascii_;
		
	void write_line_(const std::string& ln);
	
	void write_binary_(const point_xyz&);
	void write_ascii_(const point_xyz&);
	void write_binary_(const point_full&);
	void write_ascii_(const point_full&);


public:
	explicit ply_exporter(
		const std::string& filename,
		bool full = true,
		bool ascii = false,
		line_delimitor ld = line_delimitor::LF
	);
	~ply_exporter();
	
	void write(const ndarray_view<1, const point_xyz>&);
	void write(const ndarray_view<1, const point_xyzrgb>&);
	void write(const ndarray_view<1, const point_full>&);
	void close();
};

}
