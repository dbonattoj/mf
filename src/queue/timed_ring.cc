#include "timed_ring.h"

namespace mf {

void timed_ring::initialize() {
	last_write_time_ = -1;
	base::initialize();
}


void timed_ring::end_write(time_unit written_duration) {
	base::end_write(written_duration);
	last_write_time_ += written_duration;
}


time_span timed_ring::readable_time_span() const {
	return time_span(
		last_write_time_ + 1 - base::readable_duration(),
		last_write_time_ + 1
	);
}


time_span timed_ring::writable_time_span() const {
	return time_span(
		last_write_time_ + 1,
		last_write_time_ + 1 + base::writable_duration()
	);
}


time_unit timed_ring::read_start_time() const noexcept {
	return last_write_time_ + 1 - base::readable_duration();
}


time_unit timed_ring::write_start_time() const noexcept {
	return last_write_time_ + 1;
}


bool timed_ring::can_write_span(time_span span) const {
	if(span.start_time() != last_write_time_ + 1) return false; // must start immediately after the last written frame 
	else if(span.duration() > base::writable_duration()) return false;
	else return true;
}


bool timed_ring::can_read_span(time_span span) const {
	time_unit skip_and_read_duration = span.start_time() - read_start_time() + span.duration();
	if(span.start_time() < read_start_time()) return false; // time span to read already passed
	else if(skip_and_read_duration > base::readable_duration()) return false;
	else return true;
}


bool timed_ring::can_skip_span(time_span span) const {
	time_unit skip_duration = span.start_time() - read_start_time() + span.duration();
	if(skip_duration > base::readable_duration()) return false;
	else return true;
}


auto timed_ring::begin_write(time_unit duration) -> section_view_type {
	auto sec = base::begin_write(duration);
	return section_view_type(sec, write_start_time());
}


auto timed_ring::begin_write_span(time_span span) -> section_view_type {
	if(span.start_time() != write_start_time()) throw sequencing_error("write span must start at write start time");
	return begin_write(span.duration());
}


auto timed_ring::begin_read(time_unit duration) -> section_view_type {
	auto sec = base::begin_read(duration);
	return section_view_type(sec, read_start_time());
}


auto timed_ring::begin_read_span(time_span span) -> section_view_type {
	if(span.duration() > base::total_duration()) throw std::invalid_argument("read duration larger than ring capacity");
	if(! can_read_span(span)) throw sequencing_error("cannot read span");
	time_unit read_start = read_start_time();
	if(span.start_time() > read_start)
		base::skip(span.start_time() - read_start);	

	assert(readable_time_span().start_time() == span.start_time());
	
	auto sec = base::begin_read(span.duration());
	return section_view_type(sec, read_start_time());
}


void timed_ring::skip_span(time_span span) {
	if(span.duration() > base::total_duration()) throw std::invalid_argument("skip duration larger than ring capacity");
	if(! can_skip_span(span)) throw sequencing_error("cannot skip span");
	
	time_unit skip_duration = span.start_time() - read_start_time() + span.duration();
	if(skip_duration > 0) base::skip(skip_duration);
}


void timed_ring::seek(time_unit t) {
	time_span readable = readable_time_span();
	if(t > 0 && readable.includes(t)) {
		base::skip(t - readable.start_time());
	} else {
		base::skip(readable.duration());
		last_write_time_ = t - 1;
	}
}


}
