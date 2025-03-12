cmake_minimum_required(VERSION 3.15)
project(ImgToAvifConverter C)

# Require C99
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(ExternalProject)

# Define where libaom will be installed.
set(LIBAOM_INSTALL_DIR ${CMAKE_BINARY_DIR}/libaom/install)

# ---------------------------
# External project: libaom (using CMake build system)
# ---------------------------
ExternalProject_Add(libaom
    GIT_REPOSITORY    https://aomedia.googlesource.com/aom
    GIT_TAG           v3.6.0
    PREFIX            ${CMAKE_BINARY_DIR}/libaom
    CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                      -DCMAKE_BUILD_TYPE=Release
                      -DENABLE_SHARED=off
                      -DENABLE_STATIC=on
)

# ---------------------------
# External project: ffmpeg (revision n7.1.1)
# ---------------------------
# FFmpeg will be configured to use the libaom we just built.
ExternalProject_Add(ffmpeg
    GIT_REPOSITORY    https://git.ffmpeg.org/ffmpeg.git
    GIT_TAG           n7.1.1
    PREFIX            ${CMAKE_BINARY_DIR}/ffmpeg
    CONFIGURE_COMMAND <SOURCE_DIR>/configure 
                      --prefix=<INSTALL_DIR> 
                      --disable-shared 
                      --enable-static 
                      --disable-programs 
                      --disable-everything 
                      --enable-protocol=file 
                      --enable-demuxer=image2 
                      --enable-decoder=mjpeg 
                      --enable-encoder=libaom-av1 
                      --enable-muxer=avif 
                      --enable-libaom 
                      --extra-cflags=-I${LIBAOM_INSTALL_DIR}/include 
                      --extra-ldflags=-L${LIBAOM_INSTALL_DIR}/lib
    BUILD_COMMAND     make -j4
    INSTALL_COMMAND   make install
    DEPENDS           libaom
)


# FFmpeg's install directory is by default <PREFIX>/install.
set(FFMPEG_INSTALL_DIR ${CMAKE_BINARY_DIR}/ffmpeg/install)
set(FFMPEG_BIN "${FFMPEG_INSTALL_DIR}/bin/ffmpeg")

# ---------------------------
# Main executable (your program)
# ---------------------------
add_executable(img2avif src/main.c)
add_dependencies(img2avif ffmpeg)

# Pass the location of the built ffmpeg binary to your code
target_compile_definitions(img2avif PRIVATE FFMPEG_PATH="${FFMPEG_BIN}")
