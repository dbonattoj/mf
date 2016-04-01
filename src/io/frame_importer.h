#ifndef MF_FRAME_IMPORTER_H_
#define MF_FRAME_IMPORTER_H_

#include "../common.h"
#include "../ndarray/ndcoord.h"
#include "../ndarray/ndarray_view.h"

namespace mf {

/// Abstract base class for frame-wise importer.
/** Each frame is an ndarray<Dim, Elem> with fixed shape. Frames are read sequentially with read_frame(),
 ** and reached_end() becomes true when after last frame was read. */
template<std::size_t Dim, typename Elem>
class frame_importer {
private:
	ndsize<Dim> frame_shape_;

protected:
	frame_importer(const ndsize<Dim>& frame_shape) :
		frame_shape_(frame_shape) { }

public:
	using frame_view_type = ndarray_view<Dim, Elem>;
	using elem_type = Elem;
	constexpr static std::size_t dimension = Dim;

	virtual ~frame_importer() { }
	
	const ndsize<Dim>& frame_shape() { return frame_shape_; }
	
	virtual void read_frame(const frame_view_type&) = 0;
	virtual bool reached_end() const = 0;
};


}

#endif

