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

# Utility rule file for man.

# Include the progress variables for this target.
include CMakeFiles/man.dir/progress.make

CMakeFiles/man: nanocat.1
CMakeFiles/man: nn_errno.3
CMakeFiles/man: nn_strerror.3
CMakeFiles/man: nn_symbol.3
CMakeFiles/man: nn_symbol_info.3
CMakeFiles/man: nn_allocmsg.3
CMakeFiles/man: nn_reallocmsg.3
CMakeFiles/man: nn_freemsg.3
CMakeFiles/man: nn_socket.3
CMakeFiles/man: nn_close.3
CMakeFiles/man: nn_get_statistic.3
CMakeFiles/man: nn_getsockopt.3
CMakeFiles/man: nn_setsockopt.3
CMakeFiles/man: nn_bind.3
CMakeFiles/man: nn_connect.3
CMakeFiles/man: nn_shutdown.3
CMakeFiles/man: nn_send.3
CMakeFiles/man: nn_recv.3
CMakeFiles/man: nn_sendmsg.3
CMakeFiles/man: nn_recvmsg.3
CMakeFiles/man: nn_device.3
CMakeFiles/man: nn_cmsg.3
CMakeFiles/man: nn_poll.3
CMakeFiles/man: nn_term.3
CMakeFiles/man: nanomsg.7
CMakeFiles/man: nn_pair.7
CMakeFiles/man: nn_reqrep.7
CMakeFiles/man: nn_pubsub.7
CMakeFiles/man: nn_survey.7
CMakeFiles/man: nn_pipeline.7
CMakeFiles/man: nn_bus.7
CMakeFiles/man: nn_inproc.7
CMakeFiles/man: nn_ipc.7
CMakeFiles/man: nn_tcp.7
CMakeFiles/man: nn_ws.7
CMakeFiles/man: nn_env.7


nanocat.1: doc/nanocat.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating nanocat.1"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nanocat.1 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nanocat.adoc

nn_errno.3: doc/nn_errno.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Generating nn_errno.3"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_errno.3 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_errno.adoc

nn_strerror.3: doc/nn_strerror.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Generating nn_strerror.3"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_strerror.3 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_strerror.adoc

nn_symbol.3: doc/nn_symbol.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Generating nn_symbol.3"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_symbol.3 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_symbol.adoc

nn_symbol_info.3: doc/nn_symbol_info.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Generating nn_symbol_info.3"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_symbol_info.3 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_symbol_info.adoc

nn_allocmsg.3: doc/nn_allocmsg.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Generating nn_allocmsg.3"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_allocmsg.3 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_allocmsg.adoc

nn_reallocmsg.3: doc/nn_reallocmsg.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Generating nn_reallocmsg.3"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_reallocmsg.3 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_reallocmsg.adoc

nn_freemsg.3: doc/nn_freemsg.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Generating nn_freemsg.3"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_freemsg.3 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_freemsg.adoc

nn_socket.3: doc/nn_socket.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Generating nn_socket.3"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_socket.3 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_socket.adoc

nn_close.3: doc/nn_close.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Generating nn_close.3"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_close.3 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_close.adoc

nn_get_statistic.3: doc/nn_get_statistic.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_11) "Generating nn_get_statistic.3"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_get_statistic.3 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_get_statistic.adoc

nn_getsockopt.3: doc/nn_getsockopt.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_12) "Generating nn_getsockopt.3"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_getsockopt.3 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_getsockopt.adoc

nn_setsockopt.3: doc/nn_setsockopt.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_13) "Generating nn_setsockopt.3"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_setsockopt.3 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_setsockopt.adoc

nn_bind.3: doc/nn_bind.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_14) "Generating nn_bind.3"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_bind.3 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_bind.adoc

nn_connect.3: doc/nn_connect.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_15) "Generating nn_connect.3"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_connect.3 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_connect.adoc

nn_shutdown.3: doc/nn_shutdown.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_16) "Generating nn_shutdown.3"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_shutdown.3 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_shutdown.adoc

nn_send.3: doc/nn_send.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_17) "Generating nn_send.3"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_send.3 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_send.adoc

nn_recv.3: doc/nn_recv.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_18) "Generating nn_recv.3"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_recv.3 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_recv.adoc

nn_sendmsg.3: doc/nn_sendmsg.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_19) "Generating nn_sendmsg.3"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_sendmsg.3 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_sendmsg.adoc

nn_recvmsg.3: doc/nn_recvmsg.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_20) "Generating nn_recvmsg.3"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_recvmsg.3 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_recvmsg.adoc

nn_device.3: doc/nn_device.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_21) "Generating nn_device.3"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_device.3 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_device.adoc

nn_cmsg.3: doc/nn_cmsg.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_22) "Generating nn_cmsg.3"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_cmsg.3 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_cmsg.adoc

nn_poll.3: doc/nn_poll.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_23) "Generating nn_poll.3"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_poll.3 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_poll.adoc

nn_term.3: doc/nn_term.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_24) "Generating nn_term.3"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_term.3 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_term.adoc

nanomsg.7: doc/nanomsg.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_25) "Generating nanomsg.7"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nanomsg.7 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nanomsg.adoc

nn_pair.7: doc/nn_pair.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_26) "Generating nn_pair.7"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_pair.7 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_pair.adoc

nn_reqrep.7: doc/nn_reqrep.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_27) "Generating nn_reqrep.7"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_reqrep.7 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_reqrep.adoc

nn_pubsub.7: doc/nn_pubsub.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_28) "Generating nn_pubsub.7"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_pubsub.7 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_pubsub.adoc

nn_survey.7: doc/nn_survey.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_29) "Generating nn_survey.7"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_survey.7 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_survey.adoc

nn_pipeline.7: doc/nn_pipeline.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_30) "Generating nn_pipeline.7"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_pipeline.7 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_pipeline.adoc

nn_bus.7: doc/nn_bus.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_31) "Generating nn_bus.7"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_bus.7 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_bus.adoc

nn_inproc.7: doc/nn_inproc.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_32) "Generating nn_inproc.7"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_inproc.7 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_inproc.adoc

nn_ipc.7: doc/nn_ipc.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_33) "Generating nn_ipc.7"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_ipc.7 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_ipc.adoc

nn_tcp.7: doc/nn_tcp.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_34) "Generating nn_tcp.7"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_tcp.7 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_tcp.adoc

nn_ws.7: doc/nn_ws.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_35) "Generating nn_ws.7"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_ws.7 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_ws.adoc

nn_env.7: doc/nn_env.adoc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles --progress-num=$(CMAKE_PROGRESS_36) "Generating nn_env.7"
	/usr/bin/asciidoctor -b manpage -amanmanual='nanomsg 1.1.5-5-gaab95026' -o nn_env.7 /home/dell/ZDisk/Workspace/abc/t/nanomsg/doc/nn_env.adoc

man: CMakeFiles/man
man: nanocat.1
man: nanomsg.7
man: nn_allocmsg.3
man: nn_bind.3
man: nn_bus.7
man: nn_close.3
man: nn_cmsg.3
man: nn_connect.3
man: nn_device.3
man: nn_env.7
man: nn_errno.3
man: nn_freemsg.3
man: nn_get_statistic.3
man: nn_getsockopt.3
man: nn_inproc.7
man: nn_ipc.7
man: nn_pair.7
man: nn_pipeline.7
man: nn_poll.3
man: nn_pubsub.7
man: nn_reallocmsg.3
man: nn_recv.3
man: nn_recvmsg.3
man: nn_reqrep.7
man: nn_send.3
man: nn_sendmsg.3
man: nn_setsockopt.3
man: nn_shutdown.3
man: nn_socket.3
man: nn_strerror.3
man: nn_survey.7
man: nn_symbol.3
man: nn_symbol_info.3
man: nn_tcp.7
man: nn_term.3
man: nn_ws.7
man: CMakeFiles/man.dir/build.make

.PHONY : man

# Rule to build all files generated by this target.
CMakeFiles/man.dir/build: man

.PHONY : CMakeFiles/man.dir/build

CMakeFiles/man.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/man.dir/cmake_clean.cmake
.PHONY : CMakeFiles/man.dir/clean

CMakeFiles/man.dir/depend:
	cd /home/dell/ZDisk/Workspace/abc/t/nanomsg && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/dell/ZDisk/Workspace/abc/t/nanomsg /home/dell/ZDisk/Workspace/abc/t/nanomsg /home/dell/ZDisk/Workspace/abc/t/nanomsg /home/dell/ZDisk/Workspace/abc/t/nanomsg /home/dell/ZDisk/Workspace/abc/t/nanomsg/CMakeFiles/man.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/man.dir/depend

