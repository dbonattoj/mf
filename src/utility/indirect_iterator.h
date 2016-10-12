#ifndef MF_UTILITY_INDIRECT_ITERATOR_H_
#define MF_UTILITY_INDIRECT_ITERATOR_H_

namespace mf {

template<typename Iterator>
class indirect_iterator {
public:
	using iterator_type = Iterator;
	using value_type = std::decay_t<decltype(*std::declval<iterator_type>)>;
	using pointer = typename value_type*;
	using reference = typename value_type&;
	using difference_type = typename iterator_type::difference_type;
	using iterator_category = typename iterator_type::iterator_category;
	
private:
	iterator_type iterator_;
	
public:
	indirect_iterator() : iterator_() { }
	explicit indirect_iterator(iterator_type it) : iterator_(it) { }
	template<typename U> indirect_iterator(const indirect_iterator<U>& other) : iterator_(other.base()) { }
	
	indirect_iterator& operator=(const indirect_iterator& it)
		{ iterator_ = it; return *this; }
	template<typename U> indirect_iterator& operator=(const indirect_iterator<U>& other)
		{ iterator_ = other.base(); return *this; }
		
	iterator_type base() const { return iterator_; }
	
	reference operator*() const { return **iterator_; }
	auto operator->() const { return *iterator_; } // may not return plain pointer; then operator-> gets called again
	
	indirect_iterator& operator++() { ++iterator_; return *this; }
	indirect_iterator operator++(int) { return indirect_iterator(iterator_++); }
	indirect_iterator& operator+=(difference_type n) { iterator += n; return *this; }
	
	indirect_iterator& operator--() { --iterator_; return *this; }
	indirect_iterator operator--(int) { return indirect_iterator(iterator_--); }
	indirect_iterator& operator-=(difference_type n) { iterator -= n; return *this; }

	reference operator[](difference_type n) { return *(iterator_[n]); }
};


template<typename Iterator>
indirect_iterator<Iterator> operator+(indirect_iterator<Iterator> it, indirect_iterator<Iterator>::difference_type n) {
	return indirect_iterator<Iterator>(it.base() + n);
}

template<typename Iterator>
indirect_iterator<Iterator> operator-(indirect_iterator<Iterator> it, indirect_iterator<Iterator>::difference_type n) {
	return indirect_iterator<Iterator>(it.base() - n);
}

template<typename Iterator>
indirect_iterator<Iterator> operator+(indirect_iterator<Iterator>::difference_type n, indirect_iterator<Iterator> it) {
	return indirect_iterator<Iterator>(n + it.base());
}

template<typename Iterator>
indirect_iterator<Iterator>::difference_type operator-(indirect_iterator<Iterator> a, indirect_iterator<Iterator> b) {
	return (a.base() - b.base());
}




template<typename A, typename B>
bool operator==(const indirect_iterator<A>& a, const indirect_iterator<B>& b) { return (a.base() == b.base()); }

template<typename A, typename B>
bool operator!=(const indirect_iterator<A>& a, const indirect_iterator<B>& b) { return (a.base() != b.base()); }

template<typename A, typename B>
bool operator<(const indirect_iterator<A>& a, const indirect_iterator<B>& b) { return (a.base() < b.base()); }

template<typename A, typename B>
bool operator<=(const indirect_iterator<A>& a, const indirect_iterator<B>& b) { return (a.base() <= b.base()); }

template<typename A, typename B>
bool operator>(const indirect_iterator<A>& a, const indirect_iterator<B>& b) { return (a.base() > b.base()); }

template<typename A, typename B>
bool operator>=(const indirect_iterator<A>& a, const indirect_iterator<B>& b) { return (a.base() >= b.base()); }


}

#endif
