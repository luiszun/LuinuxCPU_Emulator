add_library(data_table STATIC data_table.cpp)
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