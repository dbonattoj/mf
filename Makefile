DEBUG := 1
TARGET := ./libmf.dylib
TEST_TARGET := ./mf_test

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


LIB_SRC := $(shell find src/. -name '*.cc')
LIB_OBJ := $(patsubst src/%.cc,build/src/%.o,$(LIB_SRC))

TEST_SRC := $(shell find test/. -name '*.cc')
TEST_OBJ := $(patsubst test/%.cc,build/test/%.o,$(TEST_SRC))
 
DEP := $(patsubst %.cc,build/%.d,$(LIB_SRC))
DEP += $(patsubst %.cc,build/%.d,$(TEST_SRC))


all : $(TARGET)
	

test : $(TEST_TARGET)
	$(TEST_TARGET)

$(TARGET) : $(LIB_OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -fPIC -shared -o $@ $(LIB_OBJ) $(LDLIBS)

$(TEST_TARGET) : $(TEST_OBJ) $(LIB_OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ) $(LIB_OBJ) $(LDLIBS)

build/%.o : %.cc
	mkdir -p $(dir $@) && \
	$(CXX) $(CXXFLAGS) -c -o $@ $< -MMD

clean :
	rm -rf ./build/


.PHONY: clean test	
	

-include $(DEP)
