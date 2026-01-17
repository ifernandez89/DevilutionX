# Install script for directory: C:/DevilutionX/build_NOW/_deps/sdl_audiolib-src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/DevilutionX")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "C:/mingw64/bin/objdump.exe")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "C:/Program Files (x86)/DevilutionX/lib/libSDL_audiolib.a")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "C:/Program Files (x86)/DevilutionX/lib" TYPE STATIC_LIBRARY FILES "C:/DevilutionX/build_NOW/_deps/sdl_audiolib-build/libSDL_audiolib.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("C:/DevilutionX/build_NOW/_deps/sdl_audiolib-build/CMakeFiles/SDL_audiolib.dir/install-cxx-module-bmi-Release.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/SDL_audiolib" TYPE FILE FILES
    "C:/DevilutionX/build_NOW/_deps/sdl_audiolib-src/include/aulib.h"
    "C:/DevilutionX/build_NOW/_deps/sdl_audiolib-src/include/aulib_global.h"
    "C:/DevilutionX/build_NOW/aulib_export.h"
    "C:/DevilutionX/build_NOW/aulib_version.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/SDL_audiolib/Aulib" TYPE FILE FILES
    "C:/DevilutionX/build_NOW/_deps/sdl_audiolib-src/include/Aulib/Decoder.h"
    "C:/DevilutionX/build_NOW/_deps/sdl_audiolib-src/include/Aulib/Processor.h"
    "C:/DevilutionX/build_NOW/_deps/sdl_audiolib-src/include/Aulib/Resampler.h"
    "C:/DevilutionX/build_NOW/_deps/sdl_audiolib-src/include/Aulib/ResamplerSdl.h"
    "C:/DevilutionX/build_NOW/_deps/sdl_audiolib-src/include/Aulib/ResamplerSpeex.h"
    "C:/DevilutionX/build_NOW/_deps/sdl_audiolib-src/include/Aulib/Stream.h"
    "C:/DevilutionX/build_NOW/_deps/sdl_audiolib-src/include/Aulib/DecoderDrmp3.h"
    "C:/DevilutionX/build_NOW/_deps/sdl_audiolib-src/include/Aulib/DecoderDrwav.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "C:/Program Files (x86)/DevilutionX/lib/pkgconfig/SDL_audiolib.pc")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "C:/Program Files (x86)/DevilutionX/lib/pkgconfig" TYPE FILE FILES "C:/DevilutionX/build_NOW/_deps/sdl_audiolib-build/SDL_audiolib.pc")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/DevilutionX/build_NOW/_deps/sdl_audiolib-build/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
