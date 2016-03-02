#ifndef MF_NDARRAY_H_
#define MF_NDARRAY_H_

#include "ndarray_view.h"
#include "ndcoord.h"
#include <memory>
#include <utility>

#define MF_NDARRAY_FUNC_(func) \
	template<typename... Args> decltype(auto) func(Args&&... args) { \
		return view().func(std::forward<Args>(args)...); \
	} \
	template<typename... Args> decltype(auto) func(Args&&... args) const { \
		return cview().func(std::forward<Args>(args)...); \
	}


namespace mf {

template<std::size_t Dim, typename T, typename Allocator = std::allocator<T>>
class ndarray : protected ndarray_view<Dim, T> {
	using base = ndarray_view<Dim, T>;

public:
	using view_type = base;
	using const_view_type = ndarray_view<Dim, const T>;

	using index_type = typename view_type::index_type;
	using coordinates_type = typename view_type::coordinates_type;
	using shape_type = typename view_type::shape_type;
	using strides_type = typename view_type::strides_type;

	using padding_type = ndsize<Dim>;

	using value_type = T;
	using pointer = T*;
	using const_pointer = const T*;
	using reference = T&;
	using const_reference = const T&;
		
	using iterator = typename view_type::iterator;
	using const_iterator = typename const_view_type::iterator;

	using raw_iterator = T*;
	using raw_const_iterator = const T*;

	constexpr static std::size_t dimension = Dim;

protected:
	Allocator allocator_;
	padding_type padding_;
	std::size_t allocated_size_ = 0;

	void allocate_();
	void deallocate_();
	
	ndarray(const shape_type& shape, const padding_type& padding, const Allocator& alloc);

	static strides_type strides_with_padding_(const shape_type& shape, const padding_type&);

public:
	explicit ndarray(const shape_type& shape, const Allocator& allocator = Allocator()) :
		ndarray(shape, padding_type(0), allocator) { }
	explicit ndarray(const const_view_type&);
	ndarray(const ndarray& arr) : ndarray(arr.cview()) { }
	~ndarray();
	
	ndarray& operator=(const const_view_type&);
	ndarray& operator=(const ndarray& arr) { return operator=(arr.cview()); }
	
	
	view_type view() { return *this; }
	const_view_type cview() const { return const_view_type(*this);  }
	const_view_type view() const { return cview(); }
	
	operator view_type () { return view(); }
	operator const_view_type() const { return cview(); }
	
	using base::index_to_coordinates;
	using base::coordinates_to_index;
	using base::coordinates_to_pointer;
	
	using base::size;
	using base::shape;
	using base::contiguous_length;
	const padding_type& padding() const noexcept { return padding_; }
	
	MF_NDARRAY_FUNC_(start);
	MF_NDARRAY_FUNC_(section);
	MF_NDARRAY_FUNC_(slice)
	MF_NDARRAY_FUNC_(operator());
	MF_NDARRAY_FUNC_(operator[]);
	MF_NDARRAY_FUNC_(at);
		
	iterator begin() { return base::begin(); }
	const_iterator begin() const { return base::begin(); }
	const_iterator cbegin() const { return base::begin(); }
	iterator end() { return base::begin(); }
	const_iterator end() const { return base::begin(); }
	const_iterator cend() const { return base::begin(); }
};


template<std::size_t Dim, typename T>
auto make_ndarray(const ndarray_view<Dim, T>& vw) {
	return ndarray<Dim, std::remove_const_t<T>>(vw);
}


}

#include "ndarray.tcc"

#endif
