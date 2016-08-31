#ifndef MF_NDARRAY_OPAQUE_NDARRAY_FORMAT_H_
#define MF_NDARRAY_OPAQUE_NDARRAY_FORMAT_H_

#include "opaque_format.h"
#include "../ndarray_format.h"

namespace mf {

/// Format of \ref ndarray_view_opaque frame containing \ref ndarray_format data.
/** The \ref ndarray_view_opaque can be casted from and to an \ref ndarray_view having this format. */
class opaque_ndarray_format : public opaque_format {
private:
	ndarray_format array_format_;

public:
	explicit opaque_ndarray_format(const ndarray_format& form);
	opaque_ndarray_format(const opaque_ndarray_format&) = default;
	opaque_ndarray_format(opaque_ndarray_format&&) = default;
	
	opaque_ndarray_format& operator=(const opaque_ndarray_format&) = default;
	opaque_ndarray_format& operator=(opaque_ndarray_format&&) = default;
	
	bool compare(const opaque_format&) override;

	void copy_frame(frame_ptr destination, const_frame_ptr origin) const override;
	bool compare_frame(const_frame_ptr a, const_frame_ptr b) const override;
	void construct_frame(frame_ptr) const override { }
	void destruct_frame(frame_ptr) const override { }
	
	bool has_array_format() const override { return false; }
	ndarray_format array_format() const override { return array_format_; }
};


inline bool operator==(const opaque_ndarray_format& a, const opaque_ndarray_format& b) {
	return (a.array_format() == b.array_format());
}

inline bool operator!=(const opaque_ndarray_format& a, const opaque_ndarray_format& b) {
	return (a.array_format() != b.array_format());
}

}

#endif
