#ifndef MF_NDARRAY_RING_H_
#define MF_NDARRAY_RING_H_

#include <functional>
#include "ndarray.h"

namespace mf {

template<std::size_t Dim, typename T>
class ndarray_ring {
private:
	ndarray<Dim + 1, T> array_;

public:
	using write_function = void(const ndarray<Dim, T>&);
	using read_function = void(const ndarray<Dim, const T>&);

	ndarray_ring(const ndsize<Dim>& spatial_shape, std::size_t time_window);
	
	void write(const std::function<write_function>&);
	void read(const std::function<read_function>&);
};

}

#include "ndarray_ring.tcc"

#endif
