# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

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

# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /home/miguel/Programs/clion-2020.2.4/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /home/miguel/Programs/clion-2020.2.4/bin/cmake/linux/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/miguel/Desktop/CAV-20-21/examples/audio

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/miguel/Desktop/CAV-20-21/examples/audio/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/wavcp.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/wavcp.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/wavcp.dir/flags.make

CMakeFiles/wavcp.dir/wavcp.cpp.o: CMakeFiles/wavcp.dir/flags.make
CMakeFiles/wavcp.dir/wavcp.cpp.o: ../wavcp.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/miguel/Desktop/CAV-20-21/examples/audio/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/wavcp.dir/wavcp.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/wavcp.dir/wavcp.cpp.o -c /home/miguel/Desktop/CAV-20-21/examples/audio/wavcp.cpp

CMakeFiles/wavcp.dir/wavcp.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/wavcp.dir/wavcp.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/miguel/Desktop/CAV-20-21/examples/audio/wavcp.cpp > CMakeFiles/wavcp.dir/wavcp.cpp.i

CMakeFiles/wavcp.dir/wavcp.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/wavcp.dir/wavcp.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/miguel/Desktop/CAV-20-21/examples/audio/wavcp.cpp -o CMakeFiles/wavcp.dir/wavcp.cpp.s

# Object files for target wavcp
wavcp_OBJECTS = \
"CMakeFiles/wavcp.dir/wavcp.cpp.o"

# External object files for target wavcp
wavcp_EXTERNAL_OBJECTS =

wavcp: CMakeFiles/wavcp.dir/wavcp.cpp.o
wavcp: CMakeFiles/wavcp.dir/build.make
wavcp: CMakeFiles/wavcp.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/miguel/Desktop/CAV-20-21/examples/audio/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable wavcp"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/wavcp.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/wavcp.dir/build: wavcp

.PHONY : CMakeFiles/wavcp.dir/build

CMakeFiles/wavcp.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/wavcp.dir/cmake_clean.cmake
.PHONY : CMakeFiles/wavcp.dir/clean

CMakeFiles/wavcp.dir/depend:
	cd /home/miguel/Desktop/CAV-20-21/examples/audio/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/miguel/Desktop/CAV-20-21/examples/audio /home/miguel/Desktop/CAV-20-21/examples/audio /home/miguel/Desktop/CAV-20-21/examples/audio/cmake-build-debug /home/miguel/Desktop/CAV-20-21/examples/audio/cmake-build-debug /home/miguel/Desktop/CAV-20-21/examples/audio/cmake-build-debug/CMakeFiles/wavcp.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/wavcp.dir/depend

