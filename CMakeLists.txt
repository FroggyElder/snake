cmake_minimum_required(VERSION 3.20)

project(snake)

# set(CMAKE_BUILD_TYPE RelWithDebInfo)

set(Obj snake)

set(CMAKE_C_COMPILER arm-linux-gcc)

add_subdirectory(${PROJECT_SOURCE_DIR}/lib/lcd)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/touch_screen)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/bmp_read)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/font)

include_directories(${PROJECT_SOURCE_DIR}/lib/lcd)
include_directories(${PROJECT_SOURCE_DIR}/lib/touch_screen)
include_directories(${PROJECT_SOURCE_DIR}/lib/bmp_read)
include_directories(${PROJECT_SOURCE_DIR}/lib/font)

add_executable(${Obj} snake.c)

target_link_libraries(${Obj} font lcd touch_screen bmp_read pthread m)

add_custom_command(
    TARGET ${Obj}
    POST_BUILD
    COMMAND scp -O ${CMAKE_CURRENT_BINARY_DIR}/${Obj} root@192.168.172.115:/root
    COMMENT "copying to arm device"
)