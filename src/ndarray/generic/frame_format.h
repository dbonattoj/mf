#ifndef MF_NDARRAY_FRAME_FORMAT_H_
#define MF_NDARRAY_FRAME_FORMAT_H_

#include <vector>
#include <stdexcept>
#include <typeinfo>
#include <typeindex>
#include "../ndarray_view.h"

namespace mf {

class frame_array_format;

/// Format of frame that is composed one or multiple arrays.
/** Consists of set of \ref frame_array_format frames with different offsets such that they do not overlap. For frame
 ** containing one array (usual case), can be copy-constructed or copy-assigned from \ref frame_array_format. */
class frame_format {
private:
	std::vector<frame_array_format> arrays_;
	std::size_t total_size_ = 0;
	std::size_t total_aligmnent_requirement_ = 1;
	
	void add_array_frame_(const frame_array_format&);

public:
	frame_format() = default;
	frame_format(const frame_array_format&);
	frame_format(const frame_format&) = default;
	frame_format(frame_format&&) = default;
	
	frame_format& operator=(const frame_array_format&);
	frame_format& operator=(const frame_format&) = default;
	frame_format& operator=(frame_format&&) = default;

	bool is_defined() const { return (total_aligmnent_requirement_ != -1); }

	const frame_array_format& place_next_array(const frame_array_format&);
	
	std::size_t frame_size() const noexcept { return total_size_; }
	std::size_t frame_alignment_requirement() const noexcept { return total_aligmnent_requirement_; }
	
	std::size_t arrays_count() const noexcept { return arrays_.size(); }
	const frame_array_format& array_at(std::ptrdiff_t index) const { return arrays_.at(index); }
};


class frame_array_format;

template<typename Elem>
frame_array_format make_frame_array_format(std::size_t, std::size_t = sizeof(Elem), std::size_t = 0);



/// Description of frame array, homogeneous array of elements.
/** Represents `elem_count` elements stored at `offset + elem_stride*i`. Also stores element type information:
 ** element size (`sizeof`) and element alignment requirement (`alignof`).
 ** Constitutes (possibly a sole instance) a \ref frame_format. */
class frame_array_format {
private:		
	std::size_t elem_count_ = 0;
	std::size_t elem_stride_ = 0;
	std::size_t offset_ = 0;

	std::size_t elem_size_ = 0;
	std::size_t elem_alignment_ = 0;

public:
	template<typename Elem>
	friend frame_array_format make_frame_array_format(std::size_t count, std::size_t stride, std::size_t offset);

	frame_array_format() = default;

	bool is_defined() const { return (elem_count_ != 0); }

	std::size_t frame_size() const noexcept { return offset_ + (elem_count_ * elem_stride_); }
	std::size_t frame_alignment_requirement() const noexcept { return elem_alignment_; }

	std::size_t elem_count() const noexcept { return elem_count_; }
	std::size_t elem_stride() const noexcept { return elem_stride_; }
	std::size_t offset() const noexcept { return offset_; }
	
	void set_offset(std::size_t off) { offset_ = off; } // TODO better interface

	std::size_t elem_size() const noexcept { return elem_size_; }
	std::size_t elem_alignment() const noexcept { return elem_alignment_; }

	std::size_t elem_padding() const noexcept { return elem_stride() - elem_size(); }
};


template<typename Elem>
frame_array_format make_frame_array_format(std::size_t count, std::size_t stride, std::size_t offset) {
	Expects(count > 0, "frame array format must have at least one element");
	Expects(is_nonzero_multiple_of(stride, alignof(Elem)));
	Expects(stride >= sizeof(Elem));
	Expects(is_multiple_of(offset, alignof(Elem)));
	
	frame_array_format format;
	format.elem_count_ = count;
	format.elem_stride_ = stride;
	format.offset_ = offset;
	format.elem_size_ = sizeof(Elem);
	format.elem_alignment_ = sizeof(Elem);
	return format;
}


template<std::size_t Dim, typename Elem>
frame_array_format format(const ndarray_view<Dim, Elem>& vw) {
	Expects(vw.has_default_strides());
	std::size_t count = vw.shape().product();
	std::size_t stride = vw.strides().back();
	return make_frame_array_format<Elem>(count, stride);
}


}

#endif

