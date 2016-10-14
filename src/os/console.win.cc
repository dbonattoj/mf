#include "os.h"
#ifdef MF_OS_WINDOWS

#include "console.h"
#include <windows.h>

namespace mf {

void set_console_text_color(console_color col) {
	HANDLE stdout = ::GetStdHandle(STD_OUTPUT_HANDLE);
	if(stdout == nullptr) return;

	WORD color_attr = 0;
	switch(col) {
		case console_color::black: color_attr = 0;
		case console_color::blue: color_attr = FOREGROUND_BLUE;
		case console_color::red: color_attr = FOREGROUND_RED;
		case console_color::green: color_attr = FOREGROUND_GREEN;
		case console_color::magenta: color_attr = FOREGROUND_RED | FOREGROUND_BLUE;
		case console_color::cyan: color_attr = FOREGROUND_GREEN | FOREGROUND_BLUE;
		case console_color::yellow: color_attr = FOREGROUND_RED | FOREGROUND_GREEN;
	}

	::SetConsoleTextAttribute(stdout, color_attr);
}


void set_console_text_style(bool bold, bool underline) {
	return;
}


void reset_console() {
	set_console_text_color(console_color::black);
}

}

#endif