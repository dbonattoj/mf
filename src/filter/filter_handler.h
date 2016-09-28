#ifndef MF_FILTER_HANDLER_H_
#define MF_FILTER_HANDLER_H_

#include "../common.h"

namespace mf { namespace flow {

class filter;
template<std::size_t Dim, typename Elem> class filter_input;
template<std::size_t Dim, typename Elem> class filter_output;
template<typename Value> class filter_parameter;
class filter_job;


/// Filter handler which performs concrete frame processing, base class.
/** Derived by application.
 ** TODO: May be called from multiple threads, derived should not have data members. */
class filter_handler {
private:
	filter* filter_ = nullptr;

public:
	template<std::size_t Dim, typename Elem> using input_type = filter_input<Dim, Elem>;
	template<std::size_t Dim, typename Elem> using output_type = filter_output<Dim, Elem>;
	template<typename Value> using parameter_type = filter_parameter<Value>;
	using job_type = filter_job;

	filter_handler() = default;

	virtual ~filter_handler() = default;
	filter_handler(const filter_handler&) = delete;
	filter_handler(filter_handler&&) = delete;
	filter_handler& operator=(const filter_handler&) = delete;
	filter_handler& operator=(filter_handler&&) = delete;
	
	void set_this_filter(filter& filt) { filter_ = &filt; }
	const filter& this_filter() const { return *filter_; }
	filter& this_filter() { return *filter_; }
	
	operator filter& () { return this_filter(); }
	operator const filter& () const { return this_filter(); }

	virtual void setup() { }
	
	virtual void pre_process(job_type&) { }
	virtual void process(job_type&) = 0;
};

}}

#endif
