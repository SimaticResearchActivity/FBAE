# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/thomas/Documents/Travail/FBAE/build/cereal-src")
  file(MAKE_DIRECTORY "/home/thomas/Documents/Travail/FBAE/build/cereal-src")
endif()
file(MAKE_DIRECTORY
  "/home/thomas/Documents/Travail/FBAE/build/cereal-download/cereal-prefix/src/cereal-build"
  "/home/thomas/Documents/Travail/FBAE/build/cereal-download/cereal-prefix"
  "/home/thomas/Documents/Travail/FBAE/build/cereal-download/cereal-prefix/tmp"
  "/home/thomas/Documents/Travail/FBAE/build/cereal-download/cereal-prefix/src/cereal-stamp"
  "/home/thomas/Documents/Travail/FBAE/build/cereal-download/cereal-prefix/src"
  "/home/thomas/Documents/Travail/FBAE/build/cereal-download/cereal-prefix/src/cereal-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/thomas/Documents/Travail/FBAE/build/cereal-download/cereal-prefix/src/cereal-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/thomas/Documents/Travail/FBAE/build/cereal-download/cereal-prefix/src/cereal-stamp${cfgdir}") # cfgdir has leading slash
endif()
