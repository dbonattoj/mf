#ifndef MF_NDARRAY_ITERATOR_H_
#define MF_NDARRAY_ITERATOR_H_

#include <iterator>

namespace mf {

template<typename Array>
class ndarray_iterator :
std::iterator<std::random_access_iterator_tag, typename Array::value_type> {
	using base = std::iterator<std::random_access_iterator_tag, typename Array::value_type>;

public:
	using typename base::value_type;
	using typename base::reference;
	using typename base::pointer;
	
	using index_type = typename Array::index_type;
	using coordinates_type = typename Array::coordinates_type;

	constexpr static std::size_t dimension = Array::dimension;

private:
	const Array* array_ = nullptr;
	pointer pointer_ = nullptr;
	index_type index_ = 0;
	std::ptrdiff_t pointer_step_;
	std::ptrdiff_t contiguous_length_;
	
	void forward_(std::ptrdiff_t);
	void backward_(std::ptrdiff_t);

public:
	ndarray_iterator() = default;
	ndarray_iterator(const Array& array, index_type index, pointer ptr);
	ndarray_iterator(const ndarray_iterator&) = default;
	
	ndarray_iterator& operator=(const ndarray_iterator&);
	
	ndarray_iterator& operator++();
	ndarray_iterator operator++(int);
	ndarray_iterator& operator--();
	ndarray_iterator operator--(int);
	
	ndarray_iterator& operator+=(std::ptrdiff_t);
	ndarray_iterator& operator-=(std::ptrdiff_t);
	
	reference operator*() const noexcept { return *pointer_; }
	pointer operator->() const noexcept { return pointer_; }
	reference operator[](std::ptrdiff_t n) const { return *(*this + n); }
	
	friend bool operator==(const ndarray_iterator& a, const ndarray_iterator& b) noexcept { return a.index() == b.index(); }
	friend bool operator!=(const ndarray_iterator& a, const ndarray_iterator& b) noexcept { return a.index() != b.index(); }
	friend bool operator<(const ndarray_iterator& a, const ndarray_iterator& b) noexcept { return a.index() < b.index(); }
	friend bool operator<=(const ndarray_iterator& a, const ndarray_iterator& b) noexcept { return a.index() <= b.index(); }
	friend bool operator>(const ndarray_iterator& a, const ndarray_iterator& b) noexcept { return a.index() > b.index(); }
	friend bool operator>=(const ndarray_iterator& a, const ndarray_iterator& b) noexcept { return a.index() >= b.index(); }
	
	friend ndarray_iterator operator+(const ndarray_iterator& it, std::ptrdiff_t n) { auto copy = it; copy += n; return copy; }
	friend ndarray_iterator operator+(std::ptrdiff_t n, const ndarray_iterator& it) { auto copy = it; copy += n; return copy; }
	friend ndarray_iterator operator-(const ndarray_iterator& it, std::ptrdiff_t n) { auto copy = it; copy -= n; return copy; }
	friend std::ptrdiff_t operator-(const ndarray_iterator& a, const ndarray_iterator& b) { return a.index() - b.index(); }
	
	index_type index() const noexcept { return index_; }
	coordinates_type coordinates() const noexcept { return array_->index_to_coordinates(index_); }
};


template<typename Array>
constexpr std::size_t ndarray_iterator<Array>::dimension;


}

#include "ndarray_iterator.tcc"

#endif
