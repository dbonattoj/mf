#include "os.h"
#ifdef MF_OS_WINDOWS

#include "console.h"
#include <windows.h>
#include <iostream>
#include <map>

namespace mf {

void set_console_style(std::ostream& str, console_color text_color, bool bold) {
	HANDLE console_handle = nullptr;
	if(&str == &std::cout) console_handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
	else if(&str == &std::cerr) console_handle = ::GetStdHandle(STD_ERROR_HANDLE);
	else return;
	
	const static std::map<console_color, WORD> color_attr = {
		{ console_color::default_color, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE },
		{ console_color::black, 0 },
		{ console_color::white, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE },
		{ console_color::blue, FOREGROUND_BLUE },
		{ console_color::red, FOREGROUND_RED },
		{ console_color::green, FOREGROUND_GREEN },
		{ console_color::magenta, FOREGROUND_RED | FOREGROUND_BLUE },
		{ console_color::cyan, FOREGROUND_GREEN | FOREGROUND_BLUE },
		{ console_color::yellow, FOREGROUND_RED | FOREGROUND_GREEN }
	};

	::SetConsoleTextAttribute(console_handle, color_attr.at(text_color));
}


void reset_console(std::ostream& str) {
	set_console_style(str, console_color::default_color, false);
}

}

#endif
