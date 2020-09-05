cmake_minimum_required(VERSION 3.4.3)

# Set the project name
project(IngenicVideoCapture)

get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -muclibc")

# Strip out symbols to make things a bit leaner
# set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")


# Tell CMake where to look for header files and libraries
include_directories(${CMAKE_SOURCE_DIR}/src/include/imp)
include_directories(${CMAKE_SOURCE_DIR}/src/include/sysutils)
include_directories(${PROJECT_SOURCE_DIR}/../../target/usr/include/h264bitstream )


link_directories(${CMAKE_SOURCE_DIR}/lib)


file(GLOB SOURCEFILES "src/*.c")

add_executable(videocapture ${SOURCEFILES})

# Link to the shared object library libimp.so
target_link_libraries(videocapture ${CMAKE_SOURCE_DIR}/lib/libimp.so )
target_link_libraries(videocapture ${CMAKE_SOURCE_DIR}/lib/libsysutils.so )
target_link_libraries(videocapture ${CMAKE_SOURCE_DIR}/lib/libalog.so )
target_link_libraries(videocapture ${PROJECT_SOURCE_DIR}/../../target/usr/lib/libh264bitstream.a )




find_package( Threads )
target_link_libraries( videocapture ${CMAKE_THREAD_LIBS_INIT} )
target_link_libraries( videocapture rt )
target_link_libraries( videocapture m )

# add_library( h264bitstream SHARED IMPORTED )
# set_property(TARGET h264bitstream PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})
# set_property(TARGET h264bitstream PROPERTY IMPORTED_LOCATION ${CMAKE_FIND_ROOT_PATH}/)





set_property(TARGET videocapture PROPERTY C_STANDARD 99)

install(TARGETS videocapture DESTINATION bin)

