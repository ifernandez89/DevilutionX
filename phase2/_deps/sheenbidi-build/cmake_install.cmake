# Install script for directory: C:/c - c++/DevilutionX/phase2/_deps/sheenbidi-src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/c - c++/DevilutionX/emsdk/upstream/emscripten/cache/sysroot")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
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
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "C:/mingw64/bin/objdump.exe")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/c - c++/DevilutionX/phase2/_deps/sheenbidi-build/libSheenBidi.a")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/SheenBidi" TYPE FILE FILES
      "C:/c - c++/DevilutionX/phase2/_deps/sheenbidi-src/Headers/SheenBidi/SheenBidi.h"
      "C:/c - c++/DevilutionX/phase2/_deps/sheenbidi-src/Headers/SheenBidi/SBAlgorithm.h"
      "C:/c - c++/DevilutionX/phase2/_deps/sheenbidi-src/Headers/SheenBidi/SBBase.h"
      "C:/c - c++/DevilutionX/phase2/_deps/sheenbidi-src/Headers/SheenBidi/SBBidiType.h"
      "C:/c - c++/DevilutionX/phase2/_deps/sheenbidi-src/Headers/SheenBidi/SBCodepoint.h"
      "C:/c - c++/DevilutionX/phase2/_deps/sheenbidi-src/Headers/SheenBidi/SBCodepointSequence.h"
      "C:/c - c++/DevilutionX/phase2/_deps/sheenbidi-src/Headers/SheenBidi/SBGeneralCategory.h"
      "C:/c - c++/DevilutionX/phase2/_deps/sheenbidi-src/Headers/SheenBidi/SBLine.h"
      "C:/c - c++/DevilutionX/phase2/_deps/sheenbidi-src/Headers/SheenBidi/SBMirrorLocator.h"
      "C:/c - c++/DevilutionX/phase2/_deps/sheenbidi-src/Headers/SheenBidi/SBParagraph.h"
      "C:/c - c++/DevilutionX/phase2/_deps/sheenbidi-src/Headers/SheenBidi/SBRun.h"
      "C:/c - c++/DevilutionX/phase2/_deps/sheenbidi-src/Headers/SheenBidi/SBScript.h"
      "C:/c - c++/DevilutionX/phase2/_deps/sheenbidi-src/Headers/SheenBidi/SBScriptLocator.h"
      "C:/c - c++/DevilutionX/phase2/_deps/sheenbidi-src/Headers/SheenBidi/SBVersion.h"
      )
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/SheenBidi/SheenBidiTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/SheenBidi/SheenBidiTargets.cmake"
         "C:/c - c++/DevilutionX/phase2/_deps/sheenbidi-build/CMakeFiles/Export/7905023c9b9275a49f0dc0d866ab487d/SheenBidiTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/SheenBidi/SheenBidiTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/SheenBidi/SheenBidiTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/SheenBidi" TYPE FILE FILES "C:/c - c++/DevilutionX/phase2/_deps/sheenbidi-build/CMakeFiles/Export/7905023c9b9275a49f0dc0d866ab487d/SheenBidiTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/SheenBidi" TYPE FILE FILES "C:/c - c++/DevilutionX/phase2/_deps/sheenbidi-build/CMakeFiles/Export/7905023c9b9275a49f0dc0d866ab487d/SheenBidiTargets-debug.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/SheenBidi" TYPE FILE FILES
    "C:/c - c++/DevilutionX/phase2/_deps/sheenbidi-build/SheenBidi/SheenBidiConfig.cmake"
    "C:/c - c++/DevilutionX/phase2/_deps/sheenbidi-build/SheenBidi/SheenBidiConfigVersion.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "C:/c - c++/DevilutionX/phase2/_deps/sheenbidi-build/sheenbidi.pc")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/c - c++/DevilutionX/phase2/_deps/sheenbidi-build/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
