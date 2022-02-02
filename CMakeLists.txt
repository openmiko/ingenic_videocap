cmake_minimum_required(VERSION 3.4.3)

set(CMAKE_VERBOSE_MAKEFILE ON)

# Set the project name
project(IngenicVideoCapture)

add_subdirectory(src/facil.io)

set_property(TARGET facil.io PROPERTY C_STANDARD 99)

get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()

# Tell CMake where to look for header files
include_directories(${CMAKE_SOURCE_DIR}/src/include)
include_directories(${CMAKE_SOURCE_DIR}/src/include/imp)
include_directories(${CMAKE_SOURCE_DIR}/src/db)
include_directories(${CMAKE_SOURCE_DIR}/src/include/sysutils)
include_directories(${CMAKE_SOURCE_DIR}/src/libbcrypt)
include_directories(${CMAKE_SOURCE_DIR}/src/libbcrypt/crypt_blowfish)
include_directories(${CMAKE_FIND_ROOT_PATH}/usr/include/h264bitstream)
include_directories(${CMAKE_SOURCE_DIR}/../../target/usr/include)

# The videocapture source files are all the *.c files minus the autonight and getimage
file(GLOB VIDEOCAPTURE_SRC_FILES "src/*.c")
list(REMOVE_ITEM VIDEOCAPTURE_SRC_FILES "${CMAKE_SOURCE_DIR}/src/autonight.c")
list(REMOVE_ITEM VIDEOCAPTURE_SRC_FILES "${CMAKE_SOURCE_DIR}/src/getimage.c")


# autonight source files
set(AUTONIGHT_SRC_FILES "src/autonight.c")

# getimage source files
set(GETIMAGE_SRC_FILES "src/getimage.c" "src/log.c")

# api source files
set(API_SRC_FILES "src/api/api.c" "src/db/db.c" "src/log.c" "src/libbcrypt/bcrypt.c" "src/libbcrypt/crypt_blowfish/crypt_gensalt.c" "src/libbcrypt/crypt_blowfish/crypt_blowfish.c" "src/libbcrypt/crypt_blowfish/wrapper.c")

message(STATUS "Source files for videocapture binary: ${VIDEOCAPTURE_SRC_FILES}")
message(STATUS "Source files for autonight binary: ${AUTONIGHT_SRC_FILES}")
message(STATUS "Source files for getimage binary: ${GETIMAGE_SRC_FILES}")
message(STATUS "Source files for api binary: ${API_SRC_FILES}")


######################
# Target Executables #
######################

add_executable(videocapture ${VIDEOCAPTURE_SRC_FILES})
add_executable(autonight ${AUTONIGHT_SRC_FILES})
add_executable(getimage ${GETIMAGE_SRC_FILES})
add_executable(api ${API_SRC_FILES})


#########################
# Linking Configuration #
#########################

find_package( Threads )
target_link_libraries( videocapture ${CMAKE_THREAD_LIBS_INIT} )
target_link_libraries( videocapture rt )
target_link_libraries( videocapture m )
target_link_libraries( videocapture sqlite3 )


# The libimp.so library links to some code in C++. However this is a C project
# so we need to include the C++ standard libraries. So find and link it.
find_library (LIBSTDCXX NAMES stdc++)
target_link_libraries(videocapture ${LIBSTDCXX})


# ALSA Sound Library
find_library (LIB_ALSA_SOUND NAMES asound)
target_link_libraries(videocapture ${LIB_ALSA_SOUND})


# Link to the Ingenic shared object libraries. Sources for this are not available.
# Not even sure what versions are being used but these seem to work
target_link_libraries(videocapture ${CMAKE_SOURCE_DIR}/lib/libimp.so )
target_link_libraries(videocapture ${CMAKE_SOURCE_DIR}/lib/libsysutils.so )
target_link_libraries(videocapture ${CMAKE_SOURCE_DIR}/lib/libalog.so )
target_link_libraries(videocapture ${CMAKE_FIND_ROOT_PATH}/usr/lib/libh264bitstream.so )
target_link_libraries(videocapture ${CMAKE_SOURCE_DIR}/lib/libalog.so )

# Link libraries used by getimage
target_link_libraries(getimage ${CMAKE_FIND_ROOT_PATH}/usr/lib/libh264bitstream.so )

# Link libraries for api
target_link_libraries( api ${LIBSTDCXX} facil.io m sqlite3 )

message(STATUS "CMAKE_THREAD_LIBS_INIT: ${CMAKE_THREAD_LIBS_INIT}")

set_property(TARGET videocapture PROPERTY C_STANDARD 99)
set_property(TARGET getimage PROPERTY C_STANDARD 99)
set_property(TARGET api PROPERTY C_STANDARD 99)

install(TARGETS videocapture DESTINATION bin)
install(TARGETS autonight DESTINATION bin)
install(TARGETS getimage DESTINATION bin)
install(TARGETS api DESTINATION bin)

# Copy the facil shared library object to /usr/lib
install(FILES ${CMAKE_SOURCE_DIR}/src/facil.io/libfacil.io.so DESTINATION lib)
