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

# detect host operating system
HOST := $(shell uname -s)


# include platform-specific Makefile
export
ifeq ($(HOST), Darwin)
    include Makefile.darwin
else ifeq ($(HOST), Linux)
    include Makefile.linux
else
    $(error Unknown host platform)
endif
