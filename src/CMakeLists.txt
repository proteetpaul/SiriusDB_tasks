# set(absl_DIR "/home/proteet/git_repos/task1/grpc/third_party/abseil-cpp/CMake")
# set(CMAKE_PREFIX_PATH "/home/proteet/.local/lib")
# Find absl package
# find_
include_directories("/home/proteet/.local/include")
link_directories("/home/proteet/.local/lib")
SET(CMAKE_PREFIX_PATH /home/proteet/.local/)
find_package(absl CONFIG REQUIRED)
find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)


find_program(_PROTOBUF_PROTOC protoc
    HINTS /home/proteet/.local/bin
)
find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin
    HINTS /home/proteet/.local/bin
)

# Proto file
get_filename_component(task1_proto "${CMAKE_SOURCE_DIR}/protos/task1.proto" ABSOLUTE)
get_filename_component(task1_proto_path "${task1_proto}" PATH)

# Generated sources
set(task1_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/task1.pb.cc")
set(task1_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/task1.pb.h")
set(task1_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/task1.grpc.pb.cc")
set(task1_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/task1.grpc.pb.h")

add_custom_command(
      OUTPUT "${task1_proto_srcs}" "${task1_proto_hdrs}" "${task1_grpc_srcs}" "${task1_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${task1_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${task1_proto}"
      DEPENDS "${task1_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_library(task1_grpc_proto
  ${task1_grpc_srcs}
  ${task1_grpc_hdrs}
  ${task1_proto_srcs}
  ${task1_proto_hdrs})

target_link_libraries(task1_grpc_proto
  absl::log
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

add_executable(server server.cpp)
add_executable(client client.cpp)

# message("${absl::log_initialize}")
target_link_libraries(server 
    task1_grpc_proto
    absl::check absl::flags absl::flags_parse 
    absl::log
    #  absl::log_initialize
    # absl::log_globals
    gRPC::grpc++
    protobuf    
)
target_link_libraries(client 
    task1_grpc_proto
    absl::check absl::flags absl::flags_parse 
    absl::log
    gRPC::grpc++
    protobuf) 