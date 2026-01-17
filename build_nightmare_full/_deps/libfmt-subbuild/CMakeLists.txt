# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 4.2.1)

# Reject any attempt to use a toolchain file. We must not use one because
# we could be downloading it here. If the CMAKE_TOOLCHAIN_FILE environment
# variable is set, the cache variable will have been initialized from it.
unset(CMAKE_TOOLCHAIN_FILE CACHE)
unset(ENV{CMAKE_TOOLCHAIN_FILE})

# We name the project and the target for the ExternalProject_Add() call
# to something that will highlight to the user what we are working on if
# something goes wrong and an error message is produced.

project(libfmt-populate NONE)



include(ExternalProject)
ExternalProject_Add(libfmt-populate
                     "UPDATE_DISCONNECTED" "False" "DOWNLOAD_EXTRACT_TIMESTAMP" "FALSE" "EXTERNALPROJECT_INTERNAL_ARGUMENT_SEPARATOR" "URL" "https://github.com/fmtlib/fmt/releases/download/12.0.0/fmt-12.0.0.zip" "URL_HASH" "SHA256=1c32293203449792bf8e94c7f6699c643887e826f2d66a80869b4f279fb07d25"
                    SOURCE_DIR          "C:/c - c++/DevilutionX/build_nightmare_full/_deps/libfmt-src"
                    BINARY_DIR          "C:/c - c++/DevilutionX/build_nightmare_full/_deps/libfmt-build"
                    CONFIGURE_COMMAND   ""
                    BUILD_COMMAND       ""
                    INSTALL_COMMAND     ""
                    TEST_COMMAND        ""
                    USES_TERMINAL_DOWNLOAD  YES
                    USES_TERMINAL_UPDATE    YES
                    USES_TERMINAL_PATCH     YES
)


