#ifndef MF_NDARRAY_H_
#define MF_NDARRAY_H_

#include "ndarray_view.h"
#include "ndcoord.h"
#include <memory>

namespace mf {

template<std::size_t Dim, typename T, typename Allocator = std::allocator<T>>
class ndarray : private ndarray_view<Dim, T> {
public:
	using view_type = base;
	using const_view_type = ndarray_view<Dim, const T>;

	using value_type = T;
	using pointer = T*;
	using const_pointer = const T*;
	using reference = T&;
	using const_reference = const T&;
	using index_type = typename view_type::index_type;
	using coordinates_type = typename view_type::coordinates_type;
	using shape_type = typename view_type::shape_type;
	using strides_type = typename view_type::strides_type;
	
	using iterator = typename view_type::iterator;
	using const_iterator = typename const_view_type::iterator;

	constexpr static std::size_t dimension = Dim;

private:
	Allocator allocator_;
	std::size_t length_;

public:
	explicit ndarray(const shape_type& shape, const Allocator& alloc = Allocator());
	ndarray(const ndarray&);
	~ndarray();
	
	view_type view() { return *this; }
	const_view_type cview() const { return const_view_type(*this);  }
	const_view_type view() const { return cview(); }
	
	using base::default_strides;
	using base::index_to_coordinates;
	using base::coordinates_to_index;
	using base::coordinates_to_pointer;
	
	using base::size;
	using base::shape;
	using base::strides;
	using base::contiguous_length;
	
	pointer start() noexcept { return base::start(); }
	const_pointer start() const noexcept { return base::start(); }
		
	template<typename... Args> auto section(const Args&... args) { return view().section(args...); }
	template<typename... Args> auto section(const Args&... args) const { return cview().section(args...); }
	
	template<typename... Args> auto slice(const Args&... args) { return view().slice(args...); }
	template<typename... Args> auto slice(const Args&... args) const { return cview().slice(args...); }
	
	auto operator[](std::ptrdiff_t c) { return view()[c]; }
	auto operator[](std::ptrdiff_t c) const { return cview()[c]; }
	
	template<typename... Args> auto operator(Args... args) { return view()(args...); }
	template<typename... Args> auto operator(Args... args) const { return cview()(args...); }
	
	const_reference at(const coordinates_type& coord) const { return base::at(coord); }
	reference at(const coordinates_type& coord) { return base::at(coord); }
	
	iterator begin() { return base::begin(); }
	const_iterator begin() const { return base::begin(); }
	const_iterator cbegin() const { return base::begin(); }
	iterator end() { return base::begin(); }
	const_iterator end() const { return base::begin(); }
	const_iterator cend() const { return base::begin(); }	
};

}

#endif
