#ifndef MF_NDARRAY_OPAQUE_FRAME_FORMAT_H_
#define MF_NDARRAY_OPAQUE_FRAME_FORMAT_H_

#include "../ndarray_format.h"
#include <memory>
#include <type_traits>

namespace mf {

/// Format of frame in \ref ndarray_view_opaque, base class.
class opaque_format {
public:
	using frame_ptr = void*;
	using const_frame_ptr = const void*;
	
	struct extracted_part {
		std::shared_ptr<const opaque_format> format;
		std::size_t offset;
	};

private:
	std::size_t frame_size_ = 0;
	std::size_t frame_alignment_requirement_ = 1;
	bool is_pod_ = true;
	bool is_contiguous_ = true;

protected:
	void set_frame_size_(std::size_t frame_sz) { frame_size_ = frame_sz; }
	void set_frame_alignment_requirement_(std::size_t frame_align) { frame_alignment_requirement_ = frame_align; }
	void set_pod_(bool pod) { is_pod_ = pod; }
	void set_contiguous_(bool cont) { is_contiguous_ = cont; }

public:
	virtual ~opaque_format() { }
	
	virtual bool compare(const opaque_format&) const = 0;
	
	std::size_t frame_size() const { return frame_size_; }
	std::size_t frame_alignment_requirement() const { return frame_alignment_requirement_; }
	bool is_pod() const { return is_pod_; }
	bool is_contiguous() const { return is_contiguous_; }
	bool is_contiguous_pod() const { return is_contiguous_ && is_pod_; }
	
	virtual void copy_frame(frame_ptr destination, const_frame_ptr origin) const = 0;
	virtual bool compare_frame(const_frame_ptr a, const_frame_ptr b) const = 0;
	virtual void construct_frame(frame_ptr) const = 0;
	virtual void destruct_frame(frame_ptr) const = 0;
	
	virtual bool has_array_format() const { return false; }
	virtual ndarray_format array_format() const { throw std::logic_error("not implemented"); }
	
	virtual bool has_parts() const { return false; }
	virtual std::size_t parts_count() const { throw std::logic_error("not implemented"); }
	virtual extracted_part extract_part(std::ptrdiff_t index) const { throw std::logic_error("not implemented"); }
};


template<typename Format>
using enable_if_derived_from_opaque_format
	= std::enable_if_t<std::is_base_of<opaque_format, std::decay_t<Format>>::value>;

}

#endif
