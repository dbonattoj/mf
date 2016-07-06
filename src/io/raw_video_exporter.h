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

#ifndef MF_IO_RAW_VIDEO_EXPORTER_H_
#define MF_IO_RAW_VIDEO_EXPORTER_H_

#include "frame_exporter.h"
#include "raw_video_frame_format.h"
#include "../elem.h"
#include "../image/image.h"
#include <string>
#include <iosfwd>

namespace mf {

template<typename Elem>
class raw_video_exporter : public frame_exporter<2, Elem> {
	using base = frame_exporter<2, Elem>;

public:
	using format_type = raw_video_frame_format<Elem>;
	using component_type = typename format_type::component_type;
	using typename base::frame_view_type;
	using typename base::frame_shape_type;

private:
	raw_video_frame_format<Elem> format_;
	std::ofstream output_;
	
	template<typename T> void write_raw_(const ndarray_view<2, T>&);
	
	image<component_type> scaled_component_frame_(std::ptrdiff_t component, const frame_view_type&) const;

	void write_frame_planar_(const frame_view_type&);
	void write_frame_interleaved_(const frame_view_type&);

public:
	raw_video_exporter(const std::string& filename, const format_type& format);

	void write_frame(const frame_view_type&) override;
	void close() override;
};

}

#include "raw_video_exporter.tcc"

#endif
