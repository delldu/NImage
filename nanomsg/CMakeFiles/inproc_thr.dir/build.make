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
include CMakeFiles/inproc_thr.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/inproc_thr.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/inproc_thr.dir/flags.make

CMakeFiles/inproc_thr.dir/perf/inproc_thr.c.o: CMakeFiles/inproc_thr.dir/flags.make
CMakeFiles/inproc_thr.dir/perf/inproc_thr.c.o: perf/inproc_thr.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/inproc_thr.dir/perf/inproc_thr.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/inproc_thr.dir/perf/inproc_thr.c.o -c /home/dell/ZDisk/Workspace/abc/t/nanomsg/perf/inproc_thr.c

CMakeFiles/inproc_thr.dir/perf/inproc_thr.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/inproc_thr.dir/perf/inproc_thr.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/dell/ZDisk/Workspace/abc/t/nanomsg/perf/inproc_thr.c > CMakeFiles/inproc_thr.dir/perf/inproc_thr.c.i

CMakeFiles/inproc_thr.dir/perf/inproc_thr.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/inproc_thr.dir/perf/inproc_thr.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/dell/ZDisk/Workspace/abc/t/nanomsg/perf/inproc_thr.c -o CMakeFiles/inproc_thr.dir/perf/inproc_thr.c.s

# Object files for target inproc_thr
inproc_thr_OBJECTS = \
"CMakeFiles/inproc_thr.dir/perf/inproc_thr.c.o"

# External object files for target inproc_thr
inproc_thr_EXTERNAL_OBJECTS =

inproc_thr: CMakeFiles/inproc_thr.dir/perf/inproc_thr.c.o
inproc_thr: CMakeFiles/inproc_thr.dir/build.make
inproc_thr: libnanomsg.so.5.1.0
inproc_thr: CMakeFiles/inproc_thr.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable inproc_thr"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/inproc_thr.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/inproc_thr.dir/build: inproc_thr

.PHONY : CMakeFiles/inproc_thr.dir/build

CMakeFiles/inproc_thr.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/inproc_thr.dir/cmake_clean.cmake
.PHONY : CMakeFiles/inproc_thr.dir/clean

CMakeFiles/inproc_thr.dir/depend:
	cd /home/dell/ZDisk/Workspace/abc/t/nanomsg && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/dell/ZDisk/Workspace/abc/t/nanomsg /home/dell/ZDisk/Workspace/abc/t/nanomsg /home/dell/ZDisk/Workspace/abc/t/nanomsg /home/dell/ZDisk/Workspace/abc/t/nanomsg /home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles/inproc_thr.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/inproc_thr.dir/depend

