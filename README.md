# LuinixCPU_Emulator

This is an emulator implemented for the Luinux CPU architecture. Defined as a mock/educational CPU, and described in the Monkey See Monkey Code texts.

This project uses CMake.

```
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build .
````

NOTE: clang hits this problem with C++20, so we C++17 is being specified
https://github.com/google/googletest/issues/3659