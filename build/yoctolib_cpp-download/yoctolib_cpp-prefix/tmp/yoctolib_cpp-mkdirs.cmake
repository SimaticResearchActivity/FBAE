# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/thomas/Documents/Travail/FBAE/build/yoctolib_cpp-src")
  file(MAKE_DIRECTORY "/home/thomas/Documents/Travail/FBAE/build/yoctolib_cpp-src")
endif()
file(MAKE_DIRECTORY
  "/home/thomas/Documents/Travail/FBAE/build/yoctolib_cpp-download/yoctolib_cpp-prefix/src/yoctolib_cpp-build"
  "/home/thomas/Documents/Travail/FBAE/build/yoctolib_cpp-download/yoctolib_cpp-prefix"
  "/home/thomas/Documents/Travail/FBAE/build/yoctolib_cpp-download/yoctolib_cpp-prefix/tmp"
  "/home/thomas/Documents/Travail/FBAE/build/yoctolib_cpp-download/yoctolib_cpp-prefix/src/yoctolib_cpp-stamp"
  "/home/thomas/Documents/Travail/FBAE/build/yoctolib_cpp-download/yoctolib_cpp-prefix/src"
  "/home/thomas/Documents/Travail/FBAE/build/yoctolib_cpp-download/yoctolib_cpp-prefix/src/yoctolib_cpp-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/thomas/Documents/Travail/FBAE/build/yoctolib_cpp-download/yoctolib_cpp-prefix/src/yoctolib_cpp-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/thomas/Documents/Travail/FBAE/build/yoctolib_cpp-download/yoctolib_cpp-prefix/src/yoctolib_cpp-stamp${cfgdir}") # cfgdir has leading slash
endif()
