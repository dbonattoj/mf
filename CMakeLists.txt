project(mf CXX)
cmake_minimum_required(VERSION 3.1)

if(UNIX)
	add_compile_options(-std=c++14)
	
	find_program(CCACHE_FOUND ccache)
	if(CCACHE_FOUND)
		set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
		set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
	endif()
endif()

enable_testing()

include_directories(SYSTEM external/include)

find_package(OpenCV REQUIRED)

file(GLOB_RECURSE LIB_SRC "src/*.cc")
add_library(mf SHARED ${LIB_SRC})
target_link_libraries(mf ${OpenCV_LIBS})

install(TARGETS mf DESTINATION ./lib)
install(DIRECTORY src/ DESTINATION ./include/mf FILES_MATCHING PATTERN "*.h" PATTERN "*.tcc")

macro(add_catch_tests target)
	foreach(source ${ARGN})
		file(READ ${source} contents)
		string(REGEX MATCHALL "TEST_CASE\\\(\"\([\^\"]+\)\", *\"\([\^\"]+\)\"" tests ${contents})
		foreach(hit ${tests})
			string(REGEX REPLACE "TEST_CASE\\\(\"\([\^\"]+\)\", *\"\([\^\"]+\)\"" "\\1" testName ${hit})
			string(REGEX REPLACE "TEST_CASE\\\(\"\([\^\"]+\)\", *\"\([\^\"]+\)\"" "\\2" testCatchLabelString ${hit})
			
			add_test(NAME ${testName} COMMAND ${target} "\"${testName}\"")
			
			string(REGEX MATCHALL "\\[\([a-z_]+\)\\]" testCatchLabels ${testCatchLabelString})			
			set(testLabels "")
			foreach(catchLabel ${testCatchLabels})
				string(REGEX REPLACE "\\[\([a-z_]+\)\\]" "\\1" label ${catchLabel})
				set(testLabels ${testLabels} ${label})
			endforeach()
			set_tests_properties(${testName} PROPERTIES LABELS "${testLabels}")
		endforeach()
	endforeach()
endmacro()

file(GLOB_RECURSE TEST_SRC "test/*.cc")
add_executable(mf_test ${TEST_SRC})
target_link_libraries(mf_test mf)
add_catch_tests(mf_test ${TEST_SRC})
