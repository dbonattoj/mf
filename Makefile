DEBUG := 0

# detect operating system
ifeq ($(OS), Windows_NT)
	UNAME := Windows
else
	UNAME := $(shell uname -s)
endif

# include platform-specific Makefile
export DEBUG

ifeq ($(UNAME), Darwin)
	include Makefile.darwin
endif
ifeq ($(UNAME), Linux)
	include Makefile.linux
endif
