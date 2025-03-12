# ImgToAvifConverter

This project demonstrates a self-contained build where:

- **CMake + Ninja** are used as the build system.
- A small **C (C99)** program located in `src/main.c` converts `img/image.jpg` to `img/image.avif`.
- The project downloads and builds **libaom** (needed for AV1 encoding) and **FFmpeg revision n7.1.1** (configured with libaom support) using CMake’s ExternalProject module.
- No system-installed FFmpeg or third-party dependencies are needed.

## Build Instructions

1. **Create a build directory**:
    ```bash
    mkdir build
    cd build
    ```

2. **Configure the project using Ninja**:
    ```bash
    cmake -G Ninja ..
    ```

3. **Build the project**:
    ```bash
    ninja
    ```

This will first download and build libaom (using its CMake configuration), then FFmpeg, and finally your `img2avif` executable.

## Running the Converter

Make sure you have an `img` directory in your project root containing `image.jpg`. Then run:
```bash
./img2avif
