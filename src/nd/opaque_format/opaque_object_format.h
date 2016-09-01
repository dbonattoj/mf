#ifndef MF_NDARRAY_OPAQUE_OBJECT_FORMAT_H_
#define MF_NDARRAY_OPAQUE_OBJECT_FORMAT_H_

#include "opaque_format.h"
#include <type_traits>

namespace mf {

/// Format of \ref ndarray_view_opaque frame containing object instance of type \a Object.
/** \a Object can be non-POD type, and it is constructed and destructed with frame.
 ** \a Object must have public default constructor, destructor, copy assignment operator and comparison operator. */
template<typename Object>
class opaque_object_format : public opaque_format {
public:	
	using object_type = Object;

	static object_type& obj(frame_ptr);
	static const object_type& obj(const_frame_ptr);

	opaque_object_format();
	
	bool compare(const opaque_format&) const override;

	void copy_frame(frame_ptr destination, const_frame_ptr origin) const override;
	bool compare_frame(const_frame_ptr a, const_frame_ptr b) const override;
	void construct_frame(frame_ptr) const override;
	void destruct_frame(frame_ptr) const override;
};

}

#include "opaque_object_format.tcc"

#endif
