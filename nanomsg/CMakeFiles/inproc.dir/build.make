# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.19

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
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/dell/ZDisk/Workspace/abc/t/nanomsg

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/dell/ZDisk/Workspace/abc/t/nanomsg

# Include any dependencies generated for this target.
include CMakeFiles/inproc.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/inproc.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/inproc.dir/flags.make

CMakeFiles/inproc.dir/tests/inproc.c.o: CMakeFiles/inproc.dir/flags.make
CMakeFiles/inproc.dir/tests/inproc.c.o: tests/inproc.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/inproc.dir/tests/inproc.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/inproc.dir/tests/inproc.c.o -c /home/dell/ZDisk/Workspace/abc/t/nanomsg/tests/inproc.c

CMakeFiles/inproc.dir/tests/inproc.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/inproc.dir/tests/inproc.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/dell/ZDisk/Workspace/abc/t/nanomsg/tests/inproc.c > CMakeFiles/inproc.dir/tests/inproc.c.i

CMakeFiles/inproc.dir/tests/inproc.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/inproc.dir/tests/inproc.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/dell/ZDisk/Workspace/abc/t/nanomsg/tests/inproc.c -o CMakeFiles/inproc.dir/tests/inproc.c.s

# Object files for target inproc
inproc_OBJECTS = \
"CMakeFiles/inproc.dir/tests/inproc.c.o"

# External object files for target inproc
inproc_EXTERNAL_OBJECTS =

inproc: CMakeFiles/inproc.dir/tests/inproc.c.o
inproc: CMakeFiles/inproc.dir/build.make
inproc: libnanomsg.so.5.1.0
inproc: CMakeFiles/inproc.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable inproc"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/inproc.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/inproc.dir/build: inproc

.PHONY : CMakeFiles/inproc.dir/build

CMakeFiles/inproc.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/inproc.dir/cmake_clean.cmake
.PHONY : CMakeFiles/inproc.dir/clean

CMakeFiles/inproc.dir/depend:
	cd /home/dell/ZDisk/Workspace/abc/t/nanomsg && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/dell/ZDisk/Workspace/abc/t/nanomsg /home/dell/ZDisk/Workspace/abc/t/nanomsg /home/dell/ZDisk/Workspace/abc/t/nanomsg /home/dell/ZDisk/Workspace/abc/t/nanomsg /home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles/inproc.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/inproc.dir/depend

