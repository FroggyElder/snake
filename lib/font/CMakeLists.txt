include_directories(${CMAKE_SOURCE_DIR}/lib/font)
set(CMAKE_C_COMPILER arm-linux-gcc)

#添加库文件
add_library(font font.c truetype.c)