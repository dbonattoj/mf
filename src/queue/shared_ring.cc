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

#include "shared_ring.h"
#include <algorithm>

namespace mf {

shared_ring::shared_ring
(const ring::frame_format_type& frm, std::size_t capacity, time_unit end_time) :
	ring_(frm, capacity, end_time) { }


void shared_ring::break_reader() {
	std::lock_guard<std::mutex> lock(mutex_);
	if(reader_state_ > 0) {
		reader_break_.store(true);
		readable_cv_.notify_one();
	}
}


void shared_ring::break_writer() {
	std::lock_guard<std::mutex> lock(mutex_);
	if(writer_state_ > 0) {
		writer_break_.store(true);
		writable_cv_.notify_one();
	}
}


auto shared_ring::begin_write(time_unit original_duration) -> section_view_type {
	Assert(original_duration > capacity());
	if(writer_state_ != idle) throw sequencing_error("writer not idle");

	std::unique_lock<std::mutex> lock(mutex_);
locked:

	// write start position
	// if writer needs to wait (during which mutex gets unlocked),
	// then reader may seek to another position and write_start will change again...
	time_unit write_start = ring_.write_start_time();

	// truncate write span if near end
	time_unit duration = original_duration;
	if(ring_.end_time() != undefined_time)
		if(write_start + duration > end_time()) duration = end_time() - write_start;

	// if duration zero (possibly because at end), return zero-length view
	if(duration == 0) return ring_.begin_write(0);

	// prevent deadlock
	// c.f. begin_read_span
	time_unit readable = ring_.readable_duration();
	if(ring_.writable_duration() < duration && readable < reader_state_)
		throw sequencing_error("deadlock detected: ring buffer reader was already waiting");

	while(ring_.writable_duration() < duration) {
		// wait for more frames to become available
		writer_state_ = duration;
		writable_cv_.wait(lock);
		
		// writable_cv_ woke up: either:
		// - 1+ frame was read
		// - reader seeked
		// - break event occured
		// - spurious wakeup
		
		// test if writer break event occured
		if(writer_break_.exchange(false)) {
			writer_state_ = idle;
			return section_view_type::null();
		}
		
		// reader may now have seeked to other time
		if(ring_.write_start_time() != write_start) {			
			// then retry with the new write position
			// only seek() changes write position (on reader thread), so no risk of repeated recursion
			
			// avoid recursive call (or loop),
			// as it would require brief unlock with incorrect writer_state_
			writer_state_ = idle;
			goto locked;
		}
	}

	// mutex_ is still locked
	writer_state_ = accessing;
	return ring_.begin_write(duration);
}


auto shared_ring::try_begin_write(time_unit original_duration) -> section_view_type {
	Assert(original_duration > capacity());
	if(writer_state_ != idle) throw sequencing_error("writer not idle");

	std::unique_lock<std::mutex> lock(mutex_);

	time_unit write_start = ring_.write_start_time();

	time_unit duration = original_duration;
	if(end_time() != -1)
		if(write_start + duration > end_time()) duration = end_time() - write_start;

	if(ring_.writable_duration() < duration) return section_view_type::null();

	section_view_type vw = ring_.begin_write(duration);

	writer_state_ = accessing;
	return vw;
}


bool shared_ring::wait_writable() {
	if(writer_state_ != idle) throw sequencing_error("writer not idle");

	std::unique_lock<std::mutex> lock(mutex_);
	
	if(ring_.writable_duration() == 0 && ring_.readable_duration() < reader_state_)
		throw sequencing_error("deadlock detected: ring buffer reader was already waiting");

	while(writable_time_span_().duration() == 0) {
		writer_state_ = 1;
		writable_cv_.wait(lock);
		
		if(writer_break_.exchange(false)) {
			writer_state_ = idle;
			return false;		
		}
	}
	
	writer_state_ = idle;
	return true;
}


void shared_ring::end_write(time_unit written_duration) {
	if(writer_state_ == idle) throw sequencing_error("was not writing");
	
	{
		std::lock_guard<std::mutex> lock(mutex_);
		ring_.end_write(written_duration);
	}
	
	writer_state_ = idle;
	
	readable_cv_.notify_one();	
	// notify even if no frame was written:
	// seek() waits for writer to finish
}


auto shared_ring::begin_read_span(time_span span) -> section_view_type {
	Assert(span.duration() <= capacity());
	if(reader_state_ != idle) throw sequencing_error("reader not idle");

	// if span does not start immediatly at first readable frame, skip frames, or seek if possible	
	seek(span.start_time());

	// read_start_time_ is only affected by this reader thread
	Assert(read_start_time_ == span.start_time());

	return begin_read(span.duration());
}


auto shared_ring::begin_read(time_unit original_duration) -> section_view_type {
	Assert(original_duration <= capacity());
	if(reader_state_ != idle) throw sequencing_error("reader not idle");

	// lock mutex for buffer state
	std::unique_lock<std::mutex> lock(mutex_);

	// truncate read duration if near end
	// if end time not defined: buffer is not seekable, and so end can only by marked by writer (end_write).
	// then if span is not readable yet, will need to recheck after waiting for additional frames
	time_unit duration = original_duration;
	if(end_time() != -1)
		if(read_start_time_ + duration > end_time()) duration = end_time() - read_start_time_;

	// if duration zero (possibly because at end), return zero view
	if(duration == 0) return ring_.begin_read(0);
	
	// prevent deadlock
	// it is not sufficient to have a single writer_state_ == waiting state:
	// need to store for how many frames the writer is waiting, and verify here if the wait is still necessary.
	if(ring_.writable_duration() < writer_state_ && ring_.readable_duration() < duration)
		throw sequencing_error("deadlock detected: ring buffer writer was already waiting");

	// wait until duration becomes readable
	while(ring_.readable_duration() < duration) {
		reader_state_ = duration;
		// wait until more frames written (mutex unlocked during wait, and relocked after)
		
		readable_cv_.wait(lock);
		
		if(reader_break_.exchange(false)) {
			reader_state_ = idle;
			return section_view_type::null();
		}
	}
	
	reader_state_ = accessing;
	return ring_.begin_read(duration);
}



auto shared_ring::try_begin_read(time_unit original_duration) -> section_view_type {
	Assert(original_duration <= capacity());
	if(reader_state_ != idle) throw sequencing_error("reader not idle");

	// lock mutex for buffer state
	std::unique_lock<std::mutex> lock(mutex_);

	// truncate read duration if near end
	time_unit duration = original_duration;
	if(end_time() != -1)
		if(read_start_time_ + duration > end_time()) duration = end_time() - read_start_time_;
	
	if(ring_.readable_duration() < duration) return section_view_type::null();
	
	section_view_type vw = ring_.begin_read(duration);
	reader_state_ = accessing;
	return vw;
}



bool shared_ring::wait_readable() {
	if(reader_state_ != idle) throw sequencing_error("read not idle");
	
	std::unique_lock<std::mutex> lock(mutex_);
	
	if(ring_.writable_duration() < writer_state_ && ring_.readable_duration() == 0)
		throw sequencing_error("deadlock detected: ring buffer writer was already waiting");

	while(ring_.readable_duration() == 0) {
		reader_state_ = 1;
		
		readable_cv_.wait(lock);
		if(reader_break_.exchange(false)) break;
	}
	
	reader_state_ = idle;
	return (ring_.readable_duration() > 0);
}


void shared_ring::end_read(time_unit read_duration) {
	if(reader_state_ != accessing) throw sequencing_error("was not reading");
	
	{
		std::lock_guard<std::mutex> lock(mutex_);
		
		ring_.end_read(read_duration);
		read_start_time_ += read_duration;
		Assert(read_start_time_ == ring_.read_start_time());	
	}
	reader_state_ = idle;
	writable_cv_.notify_one();
}


void shared_ring::skip(time_unit skip_duration) {
	Assert(skip_duration >= 0);

	// truncate to end time
	time_unit target_time = std::min(read_start_time_ + skip_duration, end_time());
	seek(target_time);
}


bool shared_ring::can_seek(time_unit target_time) const {
	return true;
}


void shared_ring::seek(time_unit t) {	
	Assert(t <= ring_.end_time());

	// no need to do anything if already at time t
	if(t == read_start_time_) return;

	// lock the mutex
	// writer will not start or end while locked, and readable time span will not change
	std::unique_lock<std::mutex> lock(mutex_);

	bool already_readable = ring_.readable_time_span().includes(t);
		
	if(!already_readable && writer_state_ == accessing) {
		// target tile span is not in buffer, and writer is currently accessing data:
		// need to let the writer finish writing first 
				
		// allow writer to write its frames
		// works because end_write sets writer_state_ before notifying readable_cv_
				
		while(writer_state_ == accessing) readable_cv_.wait(lock);
		// mutex is now locked again
		// writer now idle, or waiting if it started another begin_write() in meantime
		
		// retest if target time is now in readable span
		already_readable = ring_.readable_time_span().includes(t);
	}
	
	
	if(already_readable) {
		// target time is already in buffer: simply skip to it ("short seek")
		time_unit skip_duration = t - read_start_time_;
		ring_.skip(skip_duration);
		read_start_time_ += skip_duration;
		Assert(read_start_time_ == ring_.read_start_time());
						
	} else {
		// writer is necessarily idle or waiting now
		Assert(writer_state_ != accessing);

		// perform seek on ring buffer now ("long seek")
		ring_.seek(t);
		read_start_time_ = t;
		Assert(read_start_time_ == ring_.read_start_time());
	}

	// notify writer: seeked, and new writable frames
	// if it was idle, notification is ignored
	// if it was waiting, begin_write() notices that write position has changed
	lock.unlock();
	
	writable_cv_.notify_one();
}


time_unit shared_ring::write_start_time() const {
	// atomic, so no need to lock
	return ring_.write_start_time();
}


time_unit shared_ring::read_start_time() const {
	std::lock_guard<std::mutex> lock(mutex_);
	return ring_.read_start_time();
	
	// use atomic member, instead of non-atomic ring_.read_start_time()
	//return read_start_time_;
}


time_span shared_ring::writable_time_span_() const {
	return ring_.writable_time_span();
}


time_span shared_ring::writable_time_span() const {
	std::lock_guard<std::mutex> lock(mutex_);
	return writable_time_span_();
}


time_span shared_ring::readable_time_span() const {
	std::lock_guard<std::mutex> lock(mutex_);
	return ring_.readable_time_span();
}


bool shared_ring::writer_reached_end() const {
	return (write_start_time() == ring_.end_time());
}


bool shared_ring::reader_reached_end() const {
	return (read_start_time() == ring_.end_time());
}


}
