add_library(networkWrapper networkWrapper.cpp)
target_link_libraries(networkWrapper
	${CMAKE_THREAD_LIBS_INIT})

include($ENV{HOME}/libraries/flatbuffers/CMake/FindFlatBuffers.cmake)

FLATBUFFERS_GENERATE_C_HEADERS("BALL" "ball.fbs")

# protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ball.proto)

# include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${FLATBUFFERS_INCLUDE_DIR})
include_directories($ENV{HOME}/libraries/asio/include)

# ball binaries
add_executable(ballServer ballServer.cpp ${BALL_OUTPUTS})
target_link_libraries(ballServer networkWrapper
	${PROTOBUF_LIBRARY})

add_executable(ballClient ballClient.cpp)
target_link_libraries(ballClient networkWrapper
	${PROTOBUF_LIBRARY})
