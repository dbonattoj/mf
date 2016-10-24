project(mf CXX)
cmake_minimum_required(VERSION 3.1)

include(GenerateExportHeader)

if(UNIX)
	add_compile_options(-std=c++14)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lpthread")
	
	find_program(CCACHE_FOUND ccache)
	if(CCACHE_FOUND)
		set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
		set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
	endif()
elseif(WIN32)
	add_compile_options(/wd4018)
endif()

enable_testing()

include_directories(SYSTEM external/include)

find_package(OpenCV REQUIRED)

# library
file(GLOB_RECURSE LIB_SRC "src/*.cc")
add_library(mf STATIC ${LIB_SRC})
target_link_libraries(mf ${OpenCV_LIBS})
generate_export_header(mf
	BASE_NAME mf
	EXPORT_MACRO_NAME MF_EXPORTS
	EXPORT_FILE_NAME "${CMAKE_SOURCE_DIR}/src/mf_exports.h"
	STATIC_DEFINE SHARED_EXPORTS_BUILT_AS_STATIC
)

install(TARGETS mf DESTINATION ./lib)
install(DIRECTORY src/ DESTINATION ./include/mf FILES_MATCHING PATTERN "*.h" PATTERN "*.tcc")

# tests
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
