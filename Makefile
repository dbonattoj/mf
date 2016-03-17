# detect operating system
ifeq ($(OS), Windows_NT)
	UNAME := Windows
else
	UNAME := $(shell uname -s)
endif

ifeq ($(UNAME), Darwin)
	include Makefile.darwin
endif
ifeq ($(UNAME), Linux)
	include Makefile.linux
endif
