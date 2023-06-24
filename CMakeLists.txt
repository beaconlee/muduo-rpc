cmake_minimum_required(VERSION 3.10)

set(CMAKE_BUILD_TYPE "Debug")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)
file(GLOB PROTOBUF ${CMAKE_CURRENT_SOURCE_DIR}/src/protobuf/*.cc)
file(GLOB EXAMPLE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/example/*.cc)
file(GLOB CALLEE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/example/callee/*.cc)
file(GLOB CALLER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/example/caller/*.cc)


project(mprpc)


set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)




add_executable(callee ${SOURCES} ${EXAMPLE_SOURCES} ${CALLEE_SOURCES} ${PROTOBUF})
add_executable(caller ${SOURCES} ${EXAMPLE_SOURCES} ${CALLER_SOURCES} ${PROTOBUF})

target_link_libraries(callee protobuf muduo_net muduo_base pthread zookeeper_mt)
target_link_libraries(caller protobuf muduo_net muduo_base pthread zookeeper_mt)

