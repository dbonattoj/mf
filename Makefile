DEBUG := 1
TARGET := dist/libmf.dylib
TEST_TARGET := dist/mf_test

CXX := clang++
CXXFLAGS := -std=c++14 -Wall -I./external/include
LDFLAGS := 
LDLIBS := 


# OpenCV 3
PACKAGES := opencv
PKG_CONFIG_PATH=/usr/local/opt/opencv3/lib/pkgconfig
CXXFLAGS += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --cflags $(PACKAGES))
LDFLAGS += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs-only-L $(PACKAGES))
LDLIBS += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs-only-l --libs-only-other $(PACKAGES))

ifeq ($(DEBUG), 1)
	CXXFLAGS += -g
else
	CXXFLAGS += -O3 -DNDEBUG
endif

# library
LIB_SRC := $(shell find src/. -name '*.cc')
LIB_OBJ := $(patsubst src/%.cc,build/src/%.o,$(LIB_SRC))
DEP += $(patsubst %.cc,build/%.d,$(LIB_SRC))

# unit tests
TEST_SRC := $(shell find test/. -name '*.cc')
TEST_OBJ := $(patsubst test/%.cc,build/test/%.o,$(TEST_SRC))
TEST_SUPPORT_SRC := $(shell find test/support/. -name '*.cc')
TEST_SUPPORT_OBJ := $(patsubst test/support/%.cc,build/test/support/%.o,$(TEST_SUPPORT_SRC))
DEP += $(patsubst %.cc,build/%.d,$(TEST_SRC) $(TEST_SUPPORT_SRC))

# programs
PROG_SRC := $(shell find prog/. -maxdepth 1 -name '*.cc')
PROG := $(patsubst prog/%.cc,dist/prog/%,$(PROG_SRC))
PROG_SUPPORT_SRC := $(shell find prog/support/. -name '*.cc')
PROG_SUPPORT_OBJ := $(patsubst prog/support/%.cc,build/prog/support/%.o,$(PROG_SUPPORT_SRC))
DEP += $(patsubst %.cc,build/%.d,$(PROG_SRC) $(PROG_SUPPORT_SRC)) 


all : $(TARGET)
	

test : $(TEST_TARGET)
	$(TEST_TARGET)

prog : $(PROG)

$(TARGET) : $(LIB_OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -fPIC -shared -o $@ $(LIB_OBJ) $(LDLIBS)

$(TEST_TARGET) : $(TEST_OBJ) $(TEST_SUPPORT_OBJ) $(LIB_OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ) $(LIB_OBJ) $(LDLIBS)

dist/prog/% : build/prog/%.o $(PROG_SUPPORT_OBJ) $(TARGET)
	mkdir -p $(dir $@) && \
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

build/%.o : %.cc
	mkdir -p $(dir $@) && \
	$(CXX) $(CXXFLAGS) -c -o $@ $< -MMD

clean :
	rm -rf ./build/


.PHONY: clean test prog
	

-include $(DEP)
