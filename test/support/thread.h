#ifndef MF_TESTSUPPORT_THREAD_H_
#define MF_TESTSUPPORT_THREAD_H_

#include <catch.hpp>
#include <string>
#include <thread>
#include <utility>
#include <functional>
#include <exception>
#include <stdexcept>
#include <cstdlib>

#define MF_TEST_THREAD_RUNNER_NAME(no) __thread_runner ## no 

#define MF_TEST_THREAD() \
	::mf::test::thread_runner MF_TEST_THREAD_RUNNER_NAME(__LINE__) = [&]() -> void

#define MF_TEST_THREAD_REQUIRE(cond) \
	if( !(cond) ) throw ::mf::test::thread_runner::exception("assertion failed: `" #cond "`")

#define MF_TEST_THREAD_REQUIRE_FALSE(cond) MF_TEST_THREAD_REQUIRE(!(cond))

namespace mf { namespace test {

class thread_runner {
public:	
	class exception {
	private:
		std::string message_;
	public:
		exception(const std::string& msg) : message_(msg) { }
		const std::string& message() const noexcept { return message_; }
	};
	
private:
	std::thread thread_;
	std::string error_message_ = "";
	
	template<typename Func>
	void thread_main_(Func&& func) {
		try {
			func();
		} catch(const exception& ex) {
			error_message_ = ex.message();
		} catch(const std::exception& ex) {
			error_message_ = std::string("std::exception: ") + ex.what();
		} catch(...) {
			error_message_ = "unknown exception";
		}
		if(! error_message_.empty()) {
			std::cerr << "error in thread: " << error_message_ << std::endl;
			std::abort();
		}
	}
	
public:
	template<typename Func>
	thread_runner(Func&& func) :
		thread_([&]{ thread_main_(std::forward<Func>(func)); }) { }
	
	thread_runner(thread_runner&& runner) :
		thread_(std::move(runner.thread_)) { }
	
	~thread_runner() {
		thread_.join();
	}
};

	
}}

#endif
