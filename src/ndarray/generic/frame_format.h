#ifndef MF_NDARRAY_FRAME_FORMAT_H_
#define MF_NDARRAY_FRAME_FORMAT_H_

#include <vector>
#include <stdexcept>
#include <typeinfo>
#include <typeindex>

namespace mf { namespace flow {

class array_frame_format;

/// Format of frame in \ref ndarray_view_generic, base class.
/** Defines length (in byte) and alignment requirement of frame. If frame format is \ref composite_frame_format, also
 ** defines \ref array_frame_format of the nested frames. */
class frame_format {
protected:
	frame_format() = default;

public:
	virtual ~frame_format() = default;

	/// Length of one frame, in bytes.
	/** Not including inter-frame padding, if any. */
	virtual std::size_t frame_length() const = 0;
	
	/// Alignment requirement of frame, in bytes.
	/** Address of first byte of frame data must always be multiple of this. */
	virtual std::size_t frame_alignment_requirement() const = 0;
	
	virtual std::size_t nested_frames_count() const { return 0; }
	virtual const array_frame_format& nested_frame_at(std::ptrdiff_t index) const
		{ throw std::logic_error("no nested frames"); }
};


/// Format of frame consisting of array of elements of same type.
/** `elem_count` elements are stored in frame at `offset + elem_stride*i`. Also stored element type information:
 ** element size (`sizeof`), element alignment requirement (`alignof`). */
class array_frame_format : public frame_format {
public:
	struct elem_type_info {
		std::size_t size;
		std::size_t alignment;
	};
	
private:
	elem_type_info elem_type_;
	std::size_t elem_count_;
	std::size_t elem_stride_;
	std::size_t offset_;

public:
	template<typename Elem>
	static elem_type_info make_elem_type_info() {
		return { sizeof(Elem), alignof(Elem) };
	}

	array_frame_format(const elem_type_info&, std::size_t elem_count, std::size_t elem_stride, std::size_t offset = 0);

	std::size_t elem_count() const noexcept { return count_; }
	std::size_t elem_stride() const noexcept { return stride_; }
	std::size_t offset() const noexcept { return offset_; }
	
	std::size_t frame_length() const override { return offset_ + (elem_count_ * elem_stride_); }
	std::size_t frame_alignment_requirement() const override { return elem_alignment_; }
};


template<typename Elem>
array_frame_format make_array_frame_format(std::size_t count, std::size_t stride = sizeof(Elem), std::size_t off = 0) {
	Expects(count > 0, "array frame format must have at least one element");
	Expects(is_nonzero_multiple_of(stride, alignof(Elem)));
	Expects(stride >= sizeof(Elem));
	Expects(is_multiple_of(offset, alignof(Elem)));
	array_frame_format::elem_type_info elem_info = array_frame_format::make_elem_type_info<Elem>();
	return array_frame_format(elem_info, count, stride, offset);
}


/// Format of frame that is composed of multiple array frames.
/** Consists of set of \ref array_frame_format frames with different offsets such that they do not overlap. */
class composite_frame_format : public frame_format {
private:
	std::vector<array_frame_format> nested_frames_;
	std::size_t total_length_ = 0;
	std::size_t total_aligmnent_requirement_ = 1;
	
public:
	composite_frame_format() = default;

	void add_nested_frame(const array_frame_format&);
	const array_frame_format& place_next_nested_frame(const array_frame_format&);
	
	std::size_t frame_length() const override { return total_length_; }
	std::size_t frame_alignment_requirement() const override { return total_aligmnent_requirement_; }
	
	std::size_t nested_frames_count() const override { return nested_frames_.size(); }
	const array_frame_format& nested_frame_at(std::ptrdiff_t index) const override { return nested_frames_.at(index); }
};

}}

#endif

