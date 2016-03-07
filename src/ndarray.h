#ifndef MF_NDARRAY_H_
#define MF_NDARRAY_H_

#include "ndarray_view.h"
#include "ndcoord.h"
#include <memory>
#include <utility>

#define MF_NDARRAY_VIEW_FUNC_(func) \
	template<typename... Args> decltype(auto) func(Args&&... args) { \
		return view().func(std::forward<Args>(args)...); \
	} \
	template<typename... Args> decltype(auto) func(Args&&... args) const { \
		return cview().func(std::forward<Args>(args)...); \
	}


namespace mf {

template<std::size_t Dim, typename T, typename Allocator = std::allocator<T>>
class ndarray {
public:
	using view_type = ndarray_view<Dim, T>;
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

	constexpr static std::size_t dimension = Dim;

protected:
	Allocator allocator_;
	padding_type padding_;
	std::size_t allocated_size_ = 0;
	view_type view_;

	void allocate_();
	void deallocate_();
	
	ndarray(const shape_type& shape, const padding_type& padding, const Allocator& alloc);

	static strides_type strides_with_padding_(const shape_type& shape, const padding_type&);

public:
	explicit ndarray(const shape_type& shp, const Allocator& allocator = Allocator()) :
		ndarray(shp, padding_type(0), allocator) { }

	explicit ndarray(const const_view_type&);
	ndarray(const ndarray& arr) : ndarray(arr.cview()) { }

	~ndarray();
	
	ndarray& operator=(const const_view_type& arr);
	ndarray& operator=(const ndarray& arr) { return operator=(arr.cview()); }
	
	view_type view() { return view_; }
	const_view_type view() const { return cview(); }
	const_view_type cview() const { return const_view_type(view_);  }
	
	operator view_type () noexcept { return view(); }
	operator const_view_type () const noexcept { return cview(); }
	
	coordinates_type index_to_coordinates(const index_type& index) const { return view_.index_to_coordinates(index); }
	index_type coordinates_to_index(const coordinates_type& coord) const { return view_.coordinates_to_index(coord); }
	pointer coordinates_to_pointer(const coordinates_type& coord) { return view_.coordinates_to_pointer(coord); }
	const_pointer coordinates_to_pointer(const coordinates_type& coord) const { return view_.coordinates_to_pointer(coord); }
	
	std::size_t size() const noexcept { return view_.size(); }
	
	pointer start() noexcept { return view_.start(); }
	const_pointer start() const noexcept { return view_.start(); }
	const shape_type& shape() const noexcept { return view_.shape(); }
	const strides_type& strides() const noexcept { return view_.strides(); }
	std::size_t contiguous_length() const noexcept { return view_.contiguous_length(); }
	const padding_type& padding() const noexcept { return padding_; }

	MF_NDARRAY_VIEW_FUNC_(section);
	MF_NDARRAY_VIEW_FUNC_(slice)
	MF_NDARRAY_VIEW_FUNC_(operator());
	MF_NDARRAY_VIEW_FUNC_(operator[]);
	MF_NDARRAY_VIEW_FUNC_(at);
		
	iterator begin() { return view().begin(); }
	const_iterator begin() const { return cview().begin(); }
	const_iterator cbegin() const { return cview().begin(); }
	iterator end() { return view().end(); }
	const_iterator end() const { return cview().end(); }
	const_iterator cend() const { return cview().end(); }
};


template<std::size_t Dim, typename T>
auto make_ndarray(const ndarray_view<Dim, T>& vw) {
	return ndarray<Dim, std::remove_const_t<T>>(vw);
}


}

#include "ndarray.tcc"

#endif
