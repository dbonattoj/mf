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

#ifndef MF_NDARRAY_OPAQUE_MULTI_NDARRAY_FORMAT_H_
#define MF_NDARRAY_OPAQUE_MULTI_NDARRAY_FORMAT_H_

#include "opaque_format.h"
#include "../ndarray_format.h"
#include <vector>

namespace mf {

/// Format of \ref ndarray_view_opaque frame containing multiple parts with \ref ndarray_format data.
/** The parts are adjacent in memory, with the required padding inserted automatically between parts, and at the end.
 ** The \ref ndarray_view_opaque can be casted from and to an \ref ndarray_view_opaque with \ref opaque_ndarray_format
 ** which covers only a single part. */
class opaque_multi_ndarray_format : public opaque_format {
public:
	struct part {
		ndarray_format format;
		std::size_t offset;
	};
	
	friend bool operator==(const part& a, const part& b) { return (a.offset == b.offset) && (a.format == b.format); }
	friend bool operator!=(const part& a, const part& b) { return (a.offset != b.offset) || (a.format != b.format); }

private:
	std::vector<part> parts_;		

protected:
	virtual void readjust_for_added_part_(const part& new_part, bool has_padding);
	std::size_t frame_size_with_end_padding_(std::size_t frame_size_without_end_padding) const;

public:
	opaque_multi_ndarray_format();
	opaque_multi_ndarray_format(const opaque_multi_ndarray_format&) = default;
	opaque_multi_ndarray_format(opaque_multi_ndarray_format&&) = default;

	opaque_multi_ndarray_format& operator=(const opaque_multi_ndarray_format&) = default;
	opaque_multi_ndarray_format& operator=(opaque_multi_ndarray_format&&) = default;
	
	bool compare(const opaque_format&) const override;

	const part& part_at(std::ptrdiff_t index) const { return parts_.at(index); }
	virtual const part& add_part(const ndarray_format& array_format);
	
	void copy_frame(frame_ptr destination, const_frame_ptr origin) const override;
	bool compare_frame(const_frame_ptr a, const_frame_ptr b) const override;
	void construct_frame(frame_ptr) const override { }
	void destruct_frame(frame_ptr) const override { }
	
	bool has_parts() const override { return true; }
	std::size_t parts_count() const override { return parts_.size(); }
	extracted_part extract_part(std::ptrdiff_t index) const override;

	friend bool operator==(const opaque_multi_ndarray_format& a, const opaque_multi_ndarray_format& b) {
		return (a.parts_ == b.parts_);
	}
	
	friend bool operator!=(const opaque_multi_ndarray_format& a, const opaque_multi_ndarray_format& b) {
		return (a.parts_ != b.parts_);
	}
};

}

#endif
