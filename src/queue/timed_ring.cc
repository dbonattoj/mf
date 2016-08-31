/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "timed_ring.h"

namespace mf {


constexpr time_unit timed_ring::undefined_time;
	

timed_ring::timed_ring(const format_ptr& frm, std::size_t capacity, time_unit end_time) :
	ring(frm, capacity),
	last_write_time_(-1),
	end_time_(end_time) { }


time_span timed_ring::readable_time_span() const {
	time_unit start = last_write_time_ + 1 - ring::readable_duration();
	time_unit end = last_write_time_ + 1;
	Assert_crit(start <= end);
	Assert_crit(end_time_ == undefined_time || end <= end_time_);
	return time_span(start, end);
}


time_span timed_ring::writable_time_span() const {
	time_unit start = last_write_time_ + 1;
	time_unit end = last_write_time_ + 1 + ring::writable_duration();
	if(end_time_ != undefined_time) end = std::min(end, end_time_);
	Assert_crit(start <= end);
	Assert_crit(end_time_ == undefined_time || end <= end_time_);
	return time_span(start, end);
}


time_unit timed_ring::readable_duration() const {
	return ring::readable_duration();
}


time_unit timed_ring::writable_duration() const {
	return writable_time_span().duration();
}


time_unit timed_ring::current_time() const {
	return last_write_time_;
}


time_unit timed_ring::read_start_time() const {
	return last_write_time_ + 1 - ring::readable_duration();
}


time_unit timed_ring::write_start_time() const {
	return last_write_time_ + 1;
}


auto timed_ring::begin_write(time_unit duration) -> section_view_type {
	if(duration > writable_duration()) throw sequencing_error("write duration is greater than writable duration");
	auto sec = ring::begin_write(duration);
	return section_view_type(sec, write_start_time());
}


void timed_ring::end_write(time_unit written_duration) {
	Assert_crit(written_duration <= writable_duration());
	ring::end_write(written_duration);
	last_write_time_ += written_duration;
}


bool timed_ring::writer_reached_end() const {
	return (end_time_ != undefined_time) && (write_start_time() == end_time_);
}


auto timed_ring::begin_read(time_unit duration) -> section_view_type {
	if(duration > readable_duration()) throw sequencing_error("read duration is greater than readable duration");
	auto sec = ring::begin_read(duration);
	return section_view_type(sec, read_start_time());
}


auto timed_ring::begin_read_span(time_span span) -> section_view_type {
	if(! readable_time_span().includes(span)) throw sequencing_error("read span is not readable");

	time_unit read_start = read_start_time();
	if(span.start_time() > read_start)
		ring::skip(span.start_time() - read_start);	

	Assert_crit(readable_time_span().start_time() == span.start_time());
	
	auto sec = ring::begin_read(span.duration());
	return section_view_type(sec, read_start_time());
}


void timed_ring::end_read(time_unit read_duration) {
	ring::end_read(read_duration);
}


void timed_ring::skip(time_unit skip_duration) {
	if(skip_duration > readable_duration()) throw sequencing_error("skip duration is greater than readable duration");
	ring::skip(skip_duration);
}


void timed_ring::seek(time_unit t) {
	Assert_crit(t >= 0);
	Assert_crit(end_time_ == undefined_time || t < end_time_);
	time_span readable = readable_time_span();
	if(t > 0 && readable.includes(t)) {
		ring::skip(t - readable.start_time());
	} else {
		ring::skip(readable.duration());
		last_write_time_ = t - 1;
	}
}

bool timed_ring::reader_reached_end() const {
	return (end_time_ != undefined_time) && (read_start_time() == end_time_);
}


}
