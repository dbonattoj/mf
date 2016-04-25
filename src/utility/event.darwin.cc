#include "os.h"
#ifdef MF_OS_DARWIN

#include "event.h"

#include <cerrno>
#include <limits>
#include <algorithm>
#include <system_error>
#include <vector>

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <unistd.h>
#include <poll.h>

namespace mf {

namespace {
	std::uintptr_t event_ident_ = 0;
}

event::event() : handle_(0) {
	int queue = ::kqueue();
	if(queue != -1) handle_ = queue;
	else throw std::system_error(errno, std::system_category(), "kqueue failed");
	
	struct ::kevent ev;
	EV_SET(&ev, event_ident_, EVFILT_USER, EV_ADD | EV_ONESHOT, NOTE_FFCOPY, 0, nullptr);
	int result = ::kevent(queue, &ev, 1, nullptr, 0, nullptr);
	if(result == -1) std::system_error(errno, std::system_category(), "kevent failed (initial add)");
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
	struct ::kevent ev;

	int queue = static_cast<int>(handle_);
	EV_SET(&ev, event_ident_, EVFILT_USER, 0, NOTE_TRIGGER, 0, nullptr);
	int result = ::kevent(queue, &ev, 1, nullptr, 0, nullptr);
	if(result == -1) throw std::system_error(errno, std::system_category(), "kevent failed (trigger)");
}


void event::wait() {
	struct ::kevent ev;

	int queue = static_cast<int>(handle_);
	EV_SET(&ev, 0, EVFILT_USER, 0, NOTE_FFNOP, 0, nullptr);
	int result = ::kevent(queue, nullptr, 0, &ev, 1, nullptr);
	if(result == -1) throw std::system_error(errno, std::system_category(), "kevent failed (wait)");
	
	EV_SET(&ev, event_ident_, EVFILT_USER, EV_ADD | EV_ONESHOT, NOTE_FFCOPY, 0, nullptr);
	result = ::kevent(queue, &ev, 1, nullptr, 0, nullptr);
	if(result == -1) std::system_error(errno, std::system_category(), "kevent failed (readd after wait)");
}


event* event::wait_any_(event** begin, event** end) {
	// prepare array of pollfd for poll function
	std::size_t n = end - begin;
	std::vector<::pollfd> fds(n);
	std::transform(begin, end, fds.begin(), [](event* ev) -> ::pollfd {
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
