# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "C:/c - c++/DevilutionX/build_SAFE/_deps/libfmt-src")
  file(MAKE_DIRECTORY "C:/c - c++/DevilutionX/build_SAFE/_deps/libfmt-src")
endif()
file(MAKE_DIRECTORY
  "C:/c - c++/DevilutionX/build_SAFE/_deps/libfmt-build"
  "C:/c - c++/DevilutionX/build_SAFE/_deps/libfmt-subbuild/libfmt-populate-prefix"
  "C:/c - c++/DevilutionX/build_SAFE/_deps/libfmt-subbuild/libfmt-populate-prefix/tmp"
  "C:/c - c++/DevilutionX/build_SAFE/_deps/libfmt-subbuild/libfmt-populate-prefix/src/libfmt-populate-stamp"
  "C:/c - c++/DevilutionX/build_SAFE/_deps/libfmt-subbuild/libfmt-populate-prefix/src"
  "C:/c - c++/DevilutionX/build_SAFE/_deps/libfmt-subbuild/libfmt-populate-prefix/src/libfmt-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/c - c++/DevilutionX/build_SAFE/_deps/libfmt-subbuild/libfmt-populate-prefix/src/libfmt-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/c - c++/DevilutionX/build_SAFE/_deps/libfmt-subbuild/libfmt-populate-prefix/src/libfmt-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
