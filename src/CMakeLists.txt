# Minimum CMake required
cmake_minimum_required(VERSION 3.5.1)
set(CMAKE_CXX_STANDARD 20)

# Project
project(raft)

option(USE_REDIS "use redis instead of our own key value store" OFF)

# Protobuf
set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${protobuf_VERSION}")

# Protobuf-compiler
set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)

set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)


# gRPC
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")
set(_GRPC_GRPCPP gRPC::grpc++)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)

# Proto file
get_filename_component(hw_proto "kvraft.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

# Generated sources
set(hw_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/kvraft.pb.cc")
set(hw_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/kvraft.pb.h")
set(hw_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/kvraft.grpc.pb.cc")
set(hw_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/kvraft.grpc.pb.h")
add_custom_command(
      OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${hw_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${hw_proto}"
      DEPENDS "${hw_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# redis
if(USE_REDIS)
message("USE REDIS hahah")
add_definitions(-DUSE_REDIS)
include_directories("${CMAKE_CURRENT_BINARY_DIR}/../../cs739-Replicated-Database/cpp_redis/includes")
set(_REDIS "${CMAKE_CURRENT_BINARY_DIR}/../../cs739-Replicated-Database/cpp_redis/build/lib/libcpp_redis.a")
include_directories("${CMAKE_CURRENT_BINARY_DIR}/../../cs739-Replicated-Database/cpp_redis/tacopie/includes")
set(_TACOPIE "${CMAKE_CURRENT_BINARY_DIR}/../../cs739-Replicated-Database/cpp_redis/tacopie/build/lib/libtacopie.a")
endif()
# Modify names for the source code
foreach(_target
  cli_client)
  add_executable(${_target} "${_target}.cpp"
    ${hw_proto_srcs}
    ${hw_grpc_srcs}
    kv_grpc_client.h
    kv_grpc_client.cpp)
  target_link_libraries(${_target}
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
    stdc++fs)
endforeach()


# Modify names for the source code
foreach(_target
  kvraft_grpc_server)
  add_executable(${_target} "${_target}.cpp"
    ${hw_proto_srcs}
    ${hw_grpc_srcs}
    kvraft_grpc_server.h
    key_value_store.h
    key_value_store.cpp
    log.h
    log.cpp)
    if(USE_REDIS) 
    target_link_libraries(${_target}
    ${_REDIS}
    ${_TACOPIE})
    endif()
  target_link_libraries(${_target}
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
    
    stdc++fs)
endforeach()


# Modify names for the source code
foreach(_target
  throughtput)
  add_executable(${_target} "./performance/${_target}.cpp"
    ${hw_proto_srcs}
    ${hw_grpc_srcs}
    kv_grpc_client.h
    kv_grpc_client.cpp
    ./performance/throughtput.h)
  target_link_libraries(${_target}
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
    stdc++fs)
endforeach()