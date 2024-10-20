cmake_minimum_required(VERSION 3.24)

project(my_image_rpc_project LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/GetAimRT.cmake)




find_package(Protobuf REQUIRED)


# Protobuf 生成的代码目录
set(PROTO_SRC_DIR ${CMAKE_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${PROTO_SRC_DIR})

# 生成 Protobuf 文件
add_protobuf_gencode_target_for_proto_path(
  TARGET_NAME image_pb_gencode
  PROTO_PATH ${CMAKE_CURRENT_SOURCE_DIR}/proto
  GENCODE_PATH ${PROTO_SRC_DIR}
)

# 生成 RPC 服务文件
add_protobuf_aimrt_rpc_gencode_target_for_proto_files(
  TARGET_NAME image_rpc_aimrt_rpc_gencode
  PROTO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/proto/my_image.proto
  GENCODE_PATH ${PROTO_SRC_DIR}
  DEP_PROTO_TARGETS image_pb_gencode
)

# 包含生成的代码
include_directories(${PROTO_SRC_DIR})

# 将生成的源文件作为一个库来链接到其他模块
file(GLOB_RECURSE GEN_PROTO_SRCS "${PROTO_SRC_DIR}/*.cc")

add_library(my_image_rpc_lib ${GEN_PROTO_SRCS})
target_link_libraries(my_image_rpc_lib image_rpc_aimrt_rpc_gencode )


add_subdirectory(src)

