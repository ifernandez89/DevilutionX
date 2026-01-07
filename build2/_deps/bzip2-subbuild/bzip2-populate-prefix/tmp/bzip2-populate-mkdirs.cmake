# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "C:/Diablo/DevilutionX Repo/DevilutionX/build2/_deps/bzip2-src")
  file(MAKE_DIRECTORY "C:/Diablo/DevilutionX Repo/DevilutionX/build2/_deps/bzip2-src")
endif()
file(MAKE_DIRECTORY
  "C:/Diablo/DevilutionX Repo/DevilutionX/build2/_deps/bzip2-build"
  "C:/Diablo/DevilutionX Repo/DevilutionX/build2/_deps/bzip2-subbuild/bzip2-populate-prefix"
  "C:/Diablo/DevilutionX Repo/DevilutionX/build2/_deps/bzip2-subbuild/bzip2-populate-prefix/tmp"
  "C:/Diablo/DevilutionX Repo/DevilutionX/build2/_deps/bzip2-subbuild/bzip2-populate-prefix/src/bzip2-populate-stamp"
  "C:/Diablo/DevilutionX Repo/DevilutionX/build2/_deps/bzip2-subbuild/bzip2-populate-prefix/src"
  "C:/Diablo/DevilutionX Repo/DevilutionX/build2/_deps/bzip2-subbuild/bzip2-populate-prefix/src/bzip2-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Diablo/DevilutionX Repo/DevilutionX/build2/_deps/bzip2-subbuild/bzip2-populate-prefix/src/bzip2-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Diablo/DevilutionX Repo/DevilutionX/build2/_deps/bzip2-subbuild/bzip2-populate-prefix/src/bzip2-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
