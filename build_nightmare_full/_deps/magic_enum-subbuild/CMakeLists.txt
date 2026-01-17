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

project(magic_enum-populate NONE)



include(ExternalProject)
ExternalProject_Add(magic_enum-populate
                     "UPDATE_DISCONNECTED" "False" "DOWNLOAD_EXTRACT_TIMESTAMP" "FALSE" "EXTERNALPROJECT_INTERNAL_ARGUMENT_SEPARATOR" "URL" "https://github.com/Neargye/magic_enum/archive/refs/tags/v0.9.7.tar.gz" "URL_HASH" "MD5=5afd218c48c3f7cf094889a182842a50"
                    SOURCE_DIR          "C:/c - c++/DevilutionX/build_nightmare_full/_deps/magic_enum-src"
                    BINARY_DIR          "C:/c - c++/DevilutionX/build_nightmare_full/_deps/magic_enum-build"
                    CONFIGURE_COMMAND   ""
                    BUILD_COMMAND       ""
                    INSTALL_COMMAND     ""
                    TEST_COMMAND        ""
                    USES_TERMINAL_DOWNLOAD  YES
                    USES_TERMINAL_UPDATE    YES
                    USES_TERMINAL_PATCH     YES
)


