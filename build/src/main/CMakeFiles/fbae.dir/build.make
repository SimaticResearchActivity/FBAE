# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.30

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /home/thomas/Softwares/cmake-3.30.0-linux-x86_64/bin/cmake

# The command to remove a file.
RM = /home/thomas/Softwares/cmake-3.30.0-linux-x86_64/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/thomas/Documents/Travail/FBAE

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/thomas/Documents/Travail/FBAE/build

# Include any dependencies generated for this target.
include src/main/CMakeFiles/fbae.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/main/CMakeFiles/fbae.dir/compiler_depend.make

# Include the progress variables for this target.
include src/main/CMakeFiles/fbae.dir/progress.make

# Include the compile flags for this target's objects.
include src/main/CMakeFiles/fbae.dir/flags.make

src/main/CMakeFiles/fbae.dir/main.cpp.o: src/main/CMakeFiles/fbae.dir/flags.make
src/main/CMakeFiles/fbae.dir/main.cpp.o: /home/thomas/Documents/Travail/FBAE/src/main/main.cpp
src/main/CMakeFiles/fbae.dir/main.cpp.o: src/main/CMakeFiles/fbae.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/thomas/Documents/Travail/FBAE/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/main/CMakeFiles/fbae.dir/main.cpp.o"
	cd /home/thomas/Documents/Travail/FBAE/build/src/main && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/main/CMakeFiles/fbae.dir/main.cpp.o -MF CMakeFiles/fbae.dir/main.cpp.o.d -o CMakeFiles/fbae.dir/main.cpp.o -c /home/thomas/Documents/Travail/FBAE/src/main/main.cpp

src/main/CMakeFiles/fbae.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/fbae.dir/main.cpp.i"
	cd /home/thomas/Documents/Travail/FBAE/build/src/main && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/thomas/Documents/Travail/FBAE/src/main/main.cpp > CMakeFiles/fbae.dir/main.cpp.i

src/main/CMakeFiles/fbae.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/fbae.dir/main.cpp.s"
	cd /home/thomas/Documents/Travail/FBAE/build/src/main && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/thomas/Documents/Travail/FBAE/src/main/main.cpp -o CMakeFiles/fbae.dir/main.cpp.s

# Object files for target fbae
fbae_OBJECTS = \
"CMakeFiles/fbae.dir/main.cpp.o"

# External object files for target fbae
fbae_EXTERNAL_OBJECTS =

src/main/fbae: src/main/CMakeFiles/fbae.dir/main.cpp.o
src/main/fbae: src/main/CMakeFiles/fbae.dir/build.make
src/main/fbae: src/core/liblib_fbae.a
src/main/fbae: yoctolib_cpp-src/Binaries/linux/x86_64/libyocto-static.a
src/main/fbae: /usr/local/lib/liblog4cxx.so.15.2.0
src/main/fbae: src/main/CMakeFiles/fbae.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/thomas/Documents/Travail/FBAE/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable fbae"
	cd /home/thomas/Documents/Travail/FBAE/build/src/main && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/fbae.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/main/CMakeFiles/fbae.dir/build: src/main/fbae
.PHONY : src/main/CMakeFiles/fbae.dir/build

src/main/CMakeFiles/fbae.dir/clean:
	cd /home/thomas/Documents/Travail/FBAE/build/src/main && $(CMAKE_COMMAND) -P CMakeFiles/fbae.dir/cmake_clean.cmake
.PHONY : src/main/CMakeFiles/fbae.dir/clean

src/main/CMakeFiles/fbae.dir/depend:
	cd /home/thomas/Documents/Travail/FBAE/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/thomas/Documents/Travail/FBAE /home/thomas/Documents/Travail/FBAE/src/main /home/thomas/Documents/Travail/FBAE/build /home/thomas/Documents/Travail/FBAE/build/src/main /home/thomas/Documents/Travail/FBAE/build/src/main/CMakeFiles/fbae.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : src/main/CMakeFiles/fbae.dir/depend

