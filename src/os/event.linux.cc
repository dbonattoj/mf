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
#include <sys/epoll.h>
#include <poll.h>
#include <time.h>

#include <iostream>
#include <chrono>

namespace mf {
	
namespace {
	::timespec to_timespec_(std::chrono::milliseconds ms) {
		::timespec spec;
		spec.tv_sec = ms.count() / 1000;
		spec.tv_nsec = (ms.count() % 1000) * 1000000;
		return spec;
	}
}
	

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


void event::send() {
	int fd = static_cast<int>(handle_);
	std::uint64_t cnt = 1;
	auto len = ::write(fd, static_cast<const void*>(&cnt), sizeof(std::uint64_t));
	if(len != sizeof(std::uint64_t)) throw std::system_error(errno, std::system_category(), "eventfd: write failed");
}


void event::receive() {
	int fd = static_cast<int>(handle_);
	std::uint64_t cnt = 0;
	auto len = ::read(fd, static_cast<void*>(&cnt), sizeof(std::uint64_t));
	if(len != sizeof(std::uint64_t)) throw std::system_error(errno, std::system_category(), "eventfd: read failed");
}


bool event::receive(std::chrono::milliseconds timeout) {
	::pollfd fds;
	fds.fd = static_cast<int>(handle_);
	fds.events = POLLIN;
	fds.revents = 0;
	
	int result = ::poll(&fds, 1, timeout.count());
	if(result == -1) throw std::system_error(errno, std::system_category(), "eventfd: poll failed");
	
	return (result == 1);
}


sticky_event::~sticky_event() { }


void sticky_event::send() {
	sent_ = true;
	event::send();
}


void sticky_event::receive() {
	if(sent_) return;
	
	::pollfd fds;
	fds.fd = static_cast<int>(handle_);
	fds.events = POLLIN;
	fds.revents = 0;
	
	int result = ::poll(&fds, 1, -1);
	if(result < 1) throw std::system_error(errno, std::system_category(), "eventfd: poll failed");
}


bool sticky_event::receive(std::chrono::milliseconds timeout) {
	if(sent_) return true;
	
	::pollfd fds;
	fds.fd = static_cast<int>(handle_);
	fds.events = POLLIN;
	fds.revents = 0;
	
	int result = ::poll(&fds, 1, timeout.count());
	if(result == -1) throw std::system_error(errno, std::system_category(), "eventfd: poll failed");

	return (result == 1);
}


void sticky_event::reset() {
	if(sent_) event::receive();
	sent_ = false;
}


event_id event_set::receive_any() {
	int epfd = ::epoll_create1(0);

	for(event* ev : events_) {
		int fd = static_cast<int>(ev->handle());
		
		::epoll_event epev;
		epev.events = EPOLLIN;
		epev.data.ptr = static_cast<void*>(ev);
		
		int result = ::epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &epev);
		if(result == -1) throw std::system_error(errno, std::system_category(), "epoll_ctl failed");
	}
		
	::epoll_event received_epev;
	int result = ::epoll_wait(epfd, &received_epev, 1, timeout_.count());
	
	::close(epfd);
		
	if(result == 1) {
		event* ev = reinterpret_cast<event*>(received_epev.data.ptr);
		ev->receive();
		return ev->id();
	} else if(result == 0) {
		return timeout_id;
	} else {
		throw std::system_error(errno, std::system_category(), "epoll_wait failed");
	}
}

}

#endif
