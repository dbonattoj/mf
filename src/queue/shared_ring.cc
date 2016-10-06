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
(const format_ptr& frm, std::size_t capacity) :
	ring_(frm, capacity)
{ 
	reader_keep_waiting_.test_and_set();
	writer_keep_waiting_.test_and_set();
}


void shared_ring::break_reader() {
	std::lock_guard<std::mutex> lock(mutex_);
	if(reader_is_waiting_()) {
		reader_keep_waiting_.clear();
		readable_cv_.notify_one();
	}
}


void shared_ring::break_writer() {
	std::lock_guard<std::mutex> lock(mutex_);
	if(writer_is_waiting_()) {
		writer_keep_waiting_.clear();
		writable_cv_.notify_one();
	}
}


auto shared_ring::begin_write(time_unit duration) -> section_view_type {
	Assert(duration <= capacity());
	Assert(duration > 0);
	if(writer_state_ != idle) throw sequencing_error("writer not idle");

	std::unique_lock<std::mutex> lock(mutex_);
	
	write_failure_time_ = -1;
	
	// prevent deadlock
	// c.f. begin_read_span
	time_unit readable = ring_.readable_duration();
	if(ring_.writable_duration() < duration && readable < frames_reader_waits_for_())
		throw sequencing_error("deadlock detected: ring buffer reader was already waiting");

	while(ring_.writable_duration() < duration) {
		// wait for more frames to become available
		writer_state_ = duration;
		writable_cv_.wait(lock);
		
		// writable_cv_ woke up: either:
		// - 1+ frame was read
		// - reader seeked
		// - writer break event occured
		// - spurious wakeup
		
		// test if writer break event occured
		if(! writer_keep_waiting_.test_and_set()) {
			writer_state_ = idle;
			return section_view_type::null();
		}
	}

	// mutex_ is still locked
	writer_state_ = accessing;
	return ring_.begin_write(duration);
}


auto shared_ring::try_begin_write(time_unit duration) -> section_view_type {
	Assert(duration <= capacity());
	Assert(duration > 0);
	if(writer_state_ != idle) throw sequencing_error("writer not idle");

	std::unique_lock<std::mutex> lock(mutex_);
	
	write_failure_time_ = -1;

	if(ring_.writable_duration() < duration) return section_view_type::null();

	section_view_type vw = ring_.begin_write(duration);

	writer_state_ = accessing;
	return vw;
}


bool shared_ring::wait_writable() {
	if(writer_state_ != idle) throw sequencing_error("writer not idle");

	std::unique_lock<std::mutex> lock(mutex_);
	
	if(ring_.writable_duration() == 0 && ring_.readable_duration() < frames_reader_waits_for_())
		throw sequencing_error("deadlock detected: ring buffer reader was already waiting");

	while(ring_.writable_time_span().duration() == 0) {
		writer_state_ = 1;
		writable_cv_.wait(lock);
		
		if(! writer_keep_waiting_.test_and_set()) break;
	}
	
	writer_state_ = idle;
	return (ring_.writable_duration() > 0);
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


void shared_ring::end_write_fail() {
	if(writer_state_ == idle) throw sequencing_error("was not writing");
	
	{
		std::lock_guard<std::mutex> lock(mutex_);
		ring_.end_write(0);
		write_failure_time_ = ring_.write_start_time();
	}
	
	writer_state_ = idle;
	
	readable_cv_.notify_one();	 /////// 
}


auto shared_ring::begin_read_span(time_span span) -> section_view_type {
	Assert(span.duration() <= capacity());
	Assert(span.duration() > 0);
	if(reader_state_ != idle) throw sequencing_error("reader not idle");

	// if span does not start immediatly at first readable frame, seek to new start time	
	seek(span.start_time());

	// read_start_time_ is only affected by this reader thread
	Assert(read_start_time_ == span.start_time());

	return begin_read(span.duration());
}


auto shared_ring::begin_read(time_unit duration) -> section_view_type {
	Assert(duration <= capacity());
	Assert(duration > 0);
	if(reader_state_ != idle) throw sequencing_error("reader not idle");

	// lock mutex for buffer state
	std::unique_lock<std::mutex> lock(mutex_);
	
	// prevent deadlock
	// it is not sufficient to have a single writer_state_ == waiting state:
	// need to store for how many frames the writer is waiting, and verify here if the wait is still necessary.
	if(ring_.writable_duration() < frames_writer_waits_for_() && ring_.readable_duration() < duration)
		throw sequencing_error("deadlock detected: ring buffer writer was already waiting");

	// wait until duration becomes readable
	while(ring_.readable_duration() < duration) {
		reader_state_ = duration;
		// wait until more frames written (mutex unlocked during wait, and relocked after)
		
		if(ring_.write_start_time() == write_failure_time_) {
			reader_state_ = idle;
			return section_view_type::null();
		}
		
		readable_cv_.wait(lock);
		
		if(! reader_keep_waiting_.test_and_set()) {
			reader_state_ = idle;
			return section_view_type::null();
		}
	}
	
	reader_state_ = accessing;
	return ring_.begin_read(duration);
}



auto shared_ring::try_begin_read(time_unit duration) -> section_view_type {
	Assert(duration <= capacity());
	Assert(duration > 0);
	if(reader_state_ != idle) throw sequencing_error("reader not idle");

	// lock mutex for buffer state
	std::unique_lock<std::mutex> lock(mutex_);
	
	if(ring_.readable_duration() < duration) return section_view_type::null();
	
	section_view_type vw = ring_.begin_read(duration);
	reader_state_ = accessing;
	return vw;
}



bool shared_ring::wait_readable() {
	if(reader_state_ != idle) throw sequencing_error("read not idle");
	
	std::unique_lock<std::mutex> lock(mutex_);
	
	if(ring_.writable_duration() < frames_writer_waits_for_() && ring_.readable_duration() == 0)
		throw sequencing_error("deadlock detected: ring buffer writer was already waiting");

	while(ring_.readable_duration() == 0) {
		reader_state_ = 1;
		
		if(ring_.write_start_time() == write_failure_time_) break;
		
		readable_cv_.wait(lock);
		if(! reader_keep_waiting_.test_and_set()) break;
	}
	
	reader_state_ = idle;
	return (ring_.readable_duration() > 0);
}


void shared_ring::end_read(time_unit read_duration, bool initialize_frames) {
	if(reader_state_ != accessing) throw sequencing_error("was not reading");
	
	{
		std::lock_guard<std::mutex> lock(mutex_);
		
		ring_.end_read(read_duration, initialize_frames);
		read_start_time_ += read_duration;
		Assert(read_start_time_ == ring_.read_start_time());	
	}
	reader_state_ = idle;
	writable_cv_.notify_one();
}


void shared_ring::skip(time_unit skip_duration) {
	Assert(skip_duration >= 0);
	seek(read_start_time_ + skip_duration);
}


void shared_ring::seek(time_unit t) {	
	// no need to do anything if already at time t
	if(t == read_start_time_) return;

	// lock the mutex
	// writer will not start or end while locked, and readable time span will not change
	std::unique_lock<std::mutex> lock(mutex_);

	bool already_readable = ring_.readable_time_span().includes(t);
		
	if(!already_readable && writer_state_ == accessing) {
		// target time span is not in buffer, and writer is currently accessing data:
		// need to let the writer finish writing first 
				
		// allow writer to write its frames
		// works because end_write sets writer_state_ before notifying readable_cv_
				
		MF_DEBUG_T("seek", "seeking readable_span=",ring_.readable_time_span(),"  new_t=",t," wait till writer ends" );

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
	// use atomic member, instead of non-atomic ring_.read_start_time()
	
	std::lock_guard<std::mutex> lock(mutex_);
	return ring_.read_start_time();
	
	return read_start_time_;
}


time_span shared_ring::writable_time_span() const {
	std::lock_guard<std::mutex> lock(mutex_);
	return ring_.writable_time_span();
}


time_span shared_ring::readable_time_span() const {
	std::lock_guard<std::mutex> lock(mutex_);
	return ring_.readable_time_span();
}


}
