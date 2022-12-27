include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG 58d77fa8070e8cec2dc1ed015d66b454c8d78850 # release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)

add_executable(Test 
  test_opcode.cpp
  test_memory.cpp
  test_assembler.cpp
  test_register.cpp
  test_processor.cpp
)
target_include_directories(Test PRIVATE ${SRC_INC_DIR})

target_link_libraries(Test PRIVATE
  GTest::GTest
  data_table
  Assembler
)

add_test(Test_gtest Test)
configure_file(test_nvmemory.bin ${CMAKE_CURRENT_BINARY_DIR}/test_nvmemory.bin COPYONLY)