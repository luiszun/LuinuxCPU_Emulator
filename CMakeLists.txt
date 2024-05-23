cmake_minimum_required(VERSION 3.27)
project(LuinixCPU_Project)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -Werror -O0")
# only if we want coverage 
#set(CMAKE_CXX_FLAGS "-Wall -Werror -O0 -coverage")

set(SRC_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/inc")
set(PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(PROJECT_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}")
add_subdirectory(src)
add_subdirectory(test)
enable_testing()
add_test(NAME UnitTests
	COMMAND Test
	WORKING_DIRECTORY ${UnitTest_DIR})