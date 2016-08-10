#ifndef MF_NDARRAY_FRAME_FORMAT_H_
#define MF_NDARRAY_FRAME_FORMAT_H_

#include "ndarray_view.h"

namespace mf {

/// Runtime description of data format in \ref ndarray.
/** Defines byte size and alignment requirement of element type (aka `sizeof(Elem)`, `alignof(Elem)`),
 ** number of elements in array (corresponds to `ndarray::size()`), and stride between elements in bytes.
 ** Describes data in a \ref ndarray_view, with default (possibly padded) strides.
 ** Does not define the dimensionality, shape components, and elem type. */
class ndarray_format {
private:		
	std::size_t elem_size_ = 0;
	std::size_t elem_alignment_ = 0;

	std::size_t length_ = 0;
	std::size_t stride_ = 0;

public:
	ndarray_format() = default;
	ndarray_format(std::size_t elem_size, std::size_t elem_alignment, std::size_t length, std::size_t stride) :
		elem_size_(elem_size), elem_alignment_(elem_alignment), length_(length), stride_(stride)
	{
		Assert(is_nonzero_multiple_of(stride, elem_alignment));
		Assert(stride >= elem_size);
	}
	
	ndarray_format(const ndarray_format&) = default;
	ndarray_format& operator=(const ndarray_format&) = default;

	bool is_defined() const { return (elem_size_ != 0); }

	std::size_t frame_size() const noexcept { return length_ * stride_; }
	std::size_t frame_alignment_requirement() const noexcept { return elem_alignment_; }

	std::size_t length() const noexcept { return length_; }
	std::size_t stride() const noexcept { return stride_; }
	
	std::size_t elem_size() const noexcept { return elem_size_; }
	std::size_t elem_alignment() const noexcept { return elem_alignment_; }

	std::size_t elem_padding() const noexcept { return stride() - elem_size(); }
};


bool operator==(const ndarray_format&, const ndarray_format&);
bool operator!=(const ndarray_format&, const ndarray_format&);


template<typename Elem>
ndarray_format make_ndarray_format(std::size_t length, std::size_t stride = sizeof(Elem)) {
	return ndarray_format(sizeof(Elem), alignof(Elem), length, stride);
}


template<std::size_t Tail_dim, std::size_t Dim, typename Elem>
ndarray_format tail_format(const ndarray_view<Dim, Elem>& vw) {
	Expects(vw.has_default_strides(Dim - Tail_dim));
	std::size_t count = tail<Tail_dim>(vw.shape()).product();
	std::size_t stride = vw.strides().back();
	return make_ndarray_format<Elem>(count, stride);
}


template<std::size_t Dim, typename Elem>
ndarray_format format(const ndarray_view<Dim, Elem>& vw) {
	return tail_format<Dim>(vw);
}


};

}

#endif

