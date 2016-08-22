#ifndef MF_IMAGE_VIEW_H_
#define MF_IMAGE_VIEW_H_

#include "../nd/ndarray_view.h"
#include "../opencv.h"
#include "../masked_elem.h"
#include "../nd/ndarray_view_cast.h"
#include <type_traits>

namespace mf {

/// View to two-dimensional image.
template<typename Pixel>
class image_view {
public:
	using shape_type = ndsize<2>;
	using pixel_type = Pixel;
	
	using ndarray_view_type = ndarray_view<2, pixel_type>;
	
	using cv_mat_type = cv::Mat_<std::remove_const_t<pixel_type>>;
	using cv_mat_qualified_type = std::conditional_t<
		std::is_const<pixel_type>::value, const cv_mat_type, cv_mat_type
	>;

protected:
	using const_view = image_view<std::add_const_t<pixel_type>>;
	mutable cv_mat_type mat_;

public:
	image_view();
	explicit image_view(const ndarray_view_type&);
	explicit image_view(cv_mat_qualified_type&);
	image_view(const image_view&);
	virtual ~image_view() = default;
	
	image_view& operator=(const image_view&) = delete;

	void reset(const image_view&);
	operator const_view () const { return const_view(mat_); }

	bool is_null() const { return mat_.empty(); }
	explicit operator bool () const { return ! is_null(); }
			
	shape_type shape() const;
	
	ndarray_view_type array_view() const;
	cv_mat_qualified_type& cv_mat() const { return mat_; }
};


}

#include "image_view.tcc"

#endif
