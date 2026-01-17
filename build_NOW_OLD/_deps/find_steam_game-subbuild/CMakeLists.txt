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

project(find_steam_game-populate NONE)



include(ExternalProject)
ExternalProject_Add(find_steam_game-populate
                     "UPDATE_DISCONNECTED" "False" "DOWNLOAD_EXTRACT_TIMESTAMP" "FALSE" "EXTERNALPROJECT_INTERNAL_ARGUMENT_SEPARATOR" "URL" "https://github.com/cxong/find_steam_game/archive/a2bd6273fc002214052c2ee3bd48d7c1e7d3f366.tar.gz" "URL_HASH" "MD5=a6950ce5d9ced8a259752bc2dc7f5311"
                    SOURCE_DIR          "C:/DevilutionX/build_NOW/_deps/find_steam_game-src"
                    BINARY_DIR          "C:/DevilutionX/build_NOW/_deps/find_steam_game-build"
                    CONFIGURE_COMMAND   ""
                    BUILD_COMMAND       ""
                    INSTALL_COMMAND     ""
                    TEST_COMMAND        ""
                    USES_TERMINAL_DOWNLOAD  YES
                    USES_TERMINAL_UPDATE    YES
                    USES_TERMINAL_PATCH     YES
)


