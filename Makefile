DEBUG := 1
TARGET := ./libmf.dylib
TEST_TARGET := ./test/mf

CXX := clang++
CXXFLAGS := -std=c++14 -Wall -I./external/include

ifeq ($(DEBUG), 1)
	CXXFLAGS += -g
else
	CXXFLAGS += -O3 -DNDEBUG
endif

LIB_SRC := $(shell find src/. -name '*.cc')
LIB_OBJ := $(patsubst src/%.cc,build/src/%.o,$(LIB_SRC))

TEST_SRC := $(shell find test/. -name '*.cc')
TEST_OBJ := $(patsubst test/%.cc,build/test/%.o,$(TEST_SRC))
 
DEP := $(patsubst %.cc,%.d,$(LIB_SRC))
DEP += $(patsubst %.cc,%.d,$(TEST_SRC))


all : $(TARGET)
	

test : $(TEST_TARGET)
	$(TEST_TARGET)

$(TARGET) : $(LIB_OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(LIB_OBJ) $(LDLIBS)

$(TEST_TARGET) : $(TEST_OBJ) $(LIB_OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ) $(LIB_OBJ) $(LDLIBS)

build/%.o : %.cc
	mkdir -p $(dir $@) && \
	$(CXX) $(CXXFLAGS) -c -o $@ $< -MMD

clean :
	rm -rf ./build/


-include $(DEP)
