cmake_minimum_required(VERSION 3.16)
project(LuinixCPU_Project)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 17)
set(SRC_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/inc")
add_subdirectory(src)
add_subdirectory(test)
enable_testing()
add_test(NAME UnitTests
	COMMAND Test)