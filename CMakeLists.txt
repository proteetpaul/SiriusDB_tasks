cmake_minimum_required(VERSION 3.8)
project(Task1 C CXX)

include_directories("/home/proteet/.local/include")
link_directories("/home/proteet/.local/lib")
# find_package(grpc CONFIG REQUIRED)

# add_subdirectory(grpc)
add_subdirectory(src)
# add_dependencies(src grpc)