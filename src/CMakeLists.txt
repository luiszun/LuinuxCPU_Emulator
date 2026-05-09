find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(GEN_SCRIPT ${CMAKE_SOURCE_DIR}/tools/generate_opcodes.py)
set(GEN_CSV ${CMAKE_SOURCE_DIR}/tools/instructions_full.csv)
set(GEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/operations_table.cpp)

add_custom_command(
    OUTPUT ${GEN_OUT}
    COMMAND ${Python3_EXECUTABLE} ${GEN_SCRIPT} ${GEN_CSV} ${GEN_OUT}
    DEPENDS ${GEN_SCRIPT} ${GEN_CSV}
    COMMENT "Generating operations into ${GEN_OUT}"
    VERBATIM
)
add_custom_target(generate_opcodes ALL DEPENDS ${GEN_OUT})

add_library(data_table STATIC ${GEN_OUT} ${CMAKE_SOURCE_DIR}/src/register_table.cpp)
add_dependencies(data_table generate_opcodes)

target_include_directories(data_table PRIVATE ${SRC_INC_DIR})

add_library(Assembler STATIC assembler.cpp)
target_include_directories(Assembler PRIVATE ${SRC_INC_DIR})
target_link_libraries(Assembler data_table)

add_executable(luinuxasm luinux_asm.cpp)
target_include_directories(luinuxasm PRIVATE ${SRC_INC_DIR})
target_link_libraries(luinuxasm Assembler data_table)

add_library(processor STATIC processor.cpp)
target_include_directories(processor PRIVATE ${SRC_INC_DIR})
target_link_libraries(processor data_table)

add_executable(luinuxcpu luinuxcpu.cpp)
target_include_directories(luinuxcpu PRIVATE ${SRC_INC_DIR})
target_link_libraries(luinuxcpu data_table processor)