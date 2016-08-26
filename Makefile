DEBUG ?= 0

# always set debug if building tests
ifeq ($(MAKECMDGOALS), test)
	DEBUG := 1
endif
ifeq ($(MAKECMDGOALS), build_test)
	DEBUG := 1
endif

# set build and dist dir
# separate build dirs for debug/deploy builds
DIST_DIR := dist
ifeq ($(DEBUG), 1)
	BUILD_DIR := build/debug
else
	BUILD_DIR := build/deploy
endif

# detect operating system
ifeq ($(OS), Windows_NT)
	UNAME := Windows
else
	UNAME := $(shell uname -s)
endif

# include platform-specific Makefile
export
ifeq ($(UNAME), Darwin)
	include Makefile.darwin
endif
ifeq ($(UNAME), Linux)
	include Makefile.linux
endif
