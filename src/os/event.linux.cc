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

#include "os.h"
#ifdef MF_OS_LINUX

#include "event.h"

#include <cerrno>
#include <limits>
#include <algorithm>
#include <system_error>

#include <unistd.h>
#include <sys/eventfd.h>
#include <poll.h>

#include <iostream>
#include <chrono>

namespace mf {
	

event::event() : handle_(0) {
	int flags = 0;
	int fd = ::eventfd(0, 0);
	if(fd != -1) handle_ = fd;
	else throw std::system_error(errno, std::system_category(), "eventfd failed");
}


event::event(event&& ev) : handle_(ev.handle_) {
	ev.handle_ = 0;
}


event::~event() {
	if(handle_ != 0) ::close(static_cast<int>(handle_));
}


event& event::operator=(event&& ev) {
	handle_ = ev.handle_;
	ev.handle_ = 0;
	return *this;
}


bool operator==(const event& a, const event& b) {
	return (a.handle_ == b.handle_);
}


void event::notify() {
	int fd = static_cast<int>(handle_);
	std::uint64_t cnt = 1;
	auto len = ::write(fd, static_cast<const void*>(&cnt), sizeof(std::uint64_t));
	if(len != sizeof(std::uint64_t)) throw std::system_error(errno, std::system_category(), "eventfd: write failed");
}


void event::wait() {
	int fd = static_cast<int>(handle_);
	std::uint64_t cnt = 0;
	auto len = ::read(fd, static_cast<void*>(&cnt), sizeof(std::uint64_t));
	if(len != sizeof(std::uint64_t)) throw std::system_error(errno, std::system_category(), "eventfd: read failed");
}


event* event::wait_any_(event** begin, event** end) {
	// prepare array of pollfd for poll function
	std::size_t n = end - begin;
	std::vector<::pollfd> fds(n);	
		
	std::transform(begin, end, fds.begin(), [&](event* ev) -> ::pollfd {
		::pollfd fd;
		fd.fd = static_cast<int>(ev->handle_);
		fd.events = POLLIN;
		fd.revents = 0;
		return fd;	
	});

	// poll the eventfd's
	int result = ::poll(fds.data(), n, -1);
	if(result < 1) throw std::system_error(errno, std::system_category(), "eventfd: poll failed");
	
	// find first eventfs that is readable
	event* received_event = nullptr;
	for(std::ptrdiff_t i = 0; i < fds.size(); ++i) {
		if((fds[i].revents & POLLIN) != 0) {
			received_event = *(begin + i);
			break;
		}
	}
	if(received_event == 0) std::system_error(0, std::system_category(), "eventfd: poll: no fd received POLLIN event");

	// read the eventfs, and return it
	received_event->wait();
	return received_event;
}


}

#endif
