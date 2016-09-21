add_library(networkWrapper networkWrapper.cpp)
target_link_libraries(networkWrapper ${Boost_FILESYSTEM_LIBRARY}
	${Boost_SYSTEM_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ball.proto)

include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})


# ball binaries
add_executable(ballServer ballServer.cpp ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(ballServer networkWrapper ${Boost_THREAD_LIBRARY}
	${PROTOBUF_LIBRARY})

add_executable(ballClient ballClient.cpp ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(ballClient networkWrapper ${Boost_THREAD_LIBRARY}
	${PROTOBUF_LIBRARY})
