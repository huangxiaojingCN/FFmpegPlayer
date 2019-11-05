cmake_minimum_required(VERSION 3.6.0)

# 头文件目录.
include_directories(${CMAKE_SOURCE_DIR}/include)

# 查找当前目录下所有的源文件，存放到变量 src_files 中.
file(GLOB src_files ${CMAKE_SOURCE_DIR}/*.cpp)

add_library(
  ffmpegplayer
  SHARED
  ${src_files}
)

# 指定静态库链接库的位置


target_link_libraries(
        ffmpegplayer
        log
)
