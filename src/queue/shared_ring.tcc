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

#include <vector>
#include <map>
/*
namespace mf {

template<typename Iterator>
Iterator shared_ring::wait_any_writable(Iterator begin, Iterator end, event& break_event) {
	struct locked_ring {
		shared_ring& ring;
		std::unique_lock<std::mutex> lock;
		
		explicit locked_ring(shared_ring& rng) :
			ring(rng), lock(rng.mutex_) { }
	};

	std::vector<locked_ring> locked_rings;
	
	std::vector<event*> events;
	std::map<event*, Iterator> event_rings;
		
	for(Iterator it = begin; it != end; ++it) {
		shared_ring& rng = *it;
		locked_rings.emplace_back(rng);
		
		if(rng.ring_.writable_duration() == 0 && rng.ring_.readable_duration() < rng.reader_state_)
			throw sequencing_error("deadlock detected: ring buffer reader was already waiting");
		
		if(rng.writable_time_span_().duration() > 0) return it;
		
		events.push_back(&rng.reader_idle_event_);
		events.push_back(&rng.reader_seek_event_);
		event_rings[&rng.reader_idle_event_] = it;
		event_rings[&rng.reader_seek_event_] = it;
	}
	
	events.push_back(&break_event);
	
	Iterator gotten_ring = end;
	
	do {
		for(locked_ring& lrng : locked_rings) {
			lrng.ring.writer_state_ = 1;
			lrng.lock.unlock();
		}
		
		event& ev = event::wait_any_list(events.begin(), events.end());
		
		for(locked_ring& lrng : locked_rings) {
			lrng.lock.lock();
			lrng.ring.writer_state_ = idle;
		}
		
		if(ev == break_event) return end;
		else gotten_ring = event_rings.at(&ev);
	} while(gotten_ring->get().writable_time_span_().duration() == 0);
	
	return gotten_ring;
}


}
*/
