# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.15

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


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
CMAKE_COMMAND = /snap/clion/98/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /snap/clion/98/bin/cmake/linux/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/karin/jiarchen/hw4

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/karin/jiarchen/hw4/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/hw4.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/hw4.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/hw4.dir/flags.make

CMakeFiles/hw4.dir/src/helper.c.o: CMakeFiles/hw4.dir/flags.make
CMakeFiles/hw4.dir/src/helper.c.o: ../src/helper.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/karin/jiarchen/hw4/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/hw4.dir/src/helper.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/hw4.dir/src/helper.c.o   -c /home/karin/jiarchen/hw4/src/helper.c

CMakeFiles/hw4.dir/src/helper.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/hw4.dir/src/helper.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/karin/jiarchen/hw4/src/helper.c > CMakeFiles/hw4.dir/src/helper.c.i

CMakeFiles/hw4.dir/src/helper.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/hw4.dir/src/helper.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/karin/jiarchen/hw4/src/helper.c -o CMakeFiles/hw4.dir/src/helper.c.s

CMakeFiles/hw4.dir/src/jobs.c.o: CMakeFiles/hw4.dir/flags.make
CMakeFiles/hw4.dir/src/jobs.c.o: ../src/jobs.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/karin/jiarchen/hw4/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/hw4.dir/src/jobs.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/hw4.dir/src/jobs.c.o   -c /home/karin/jiarchen/hw4/src/jobs.c

CMakeFiles/hw4.dir/src/jobs.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/hw4.dir/src/jobs.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/karin/jiarchen/hw4/src/jobs.c > CMakeFiles/hw4.dir/src/jobs.c.i

CMakeFiles/hw4.dir/src/jobs.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/hw4.dir/src/jobs.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/karin/jiarchen/hw4/src/jobs.c -o CMakeFiles/hw4.dir/src/jobs.c.s

CMakeFiles/hw4.dir/src/main.c.o: CMakeFiles/hw4.dir/flags.make
CMakeFiles/hw4.dir/src/main.c.o: ../src/main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/karin/jiarchen/hw4/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/hw4.dir/src/main.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/hw4.dir/src/main.c.o   -c /home/karin/jiarchen/hw4/src/main.c

CMakeFiles/hw4.dir/src/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/hw4.dir/src/main.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/karin/jiarchen/hw4/src/main.c > CMakeFiles/hw4.dir/src/main.c.i

CMakeFiles/hw4.dir/src/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/hw4.dir/src/main.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/karin/jiarchen/hw4/src/main.c -o CMakeFiles/hw4.dir/src/main.c.s

CMakeFiles/hw4.dir/src/sf_readline.c.o: CMakeFiles/hw4.dir/flags.make
CMakeFiles/hw4.dir/src/sf_readline.c.o: ../src/sf_readline.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/karin/jiarchen/hw4/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object CMakeFiles/hw4.dir/src/sf_readline.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/hw4.dir/src/sf_readline.c.o   -c /home/karin/jiarchen/hw4/src/sf_readline.c

CMakeFiles/hw4.dir/src/sf_readline.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/hw4.dir/src/sf_readline.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/karin/jiarchen/hw4/src/sf_readline.c > CMakeFiles/hw4.dir/src/sf_readline.c.i

CMakeFiles/hw4.dir/src/sf_readline.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/hw4.dir/src/sf_readline.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/karin/jiarchen/hw4/src/sf_readline.c -o CMakeFiles/hw4.dir/src/sf_readline.c.s

CMakeFiles/hw4.dir/src/task.c.o: CMakeFiles/hw4.dir/flags.make
CMakeFiles/hw4.dir/src/task.c.o: ../src/task.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/karin/jiarchen/hw4/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building C object CMakeFiles/hw4.dir/src/task.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/hw4.dir/src/task.c.o   -c /home/karin/jiarchen/hw4/src/task.c

CMakeFiles/hw4.dir/src/task.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/hw4.dir/src/task.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/karin/jiarchen/hw4/src/task.c > CMakeFiles/hw4.dir/src/task.c.i

CMakeFiles/hw4.dir/src/task.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/hw4.dir/src/task.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/karin/jiarchen/hw4/src/task.c -o CMakeFiles/hw4.dir/src/task.c.s

CMakeFiles/hw4.dir/tests/hw4_tests.c.o: CMakeFiles/hw4.dir/flags.make
CMakeFiles/hw4.dir/tests/hw4_tests.c.o: ../tests/hw4_tests.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/karin/jiarchen/hw4/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building C object CMakeFiles/hw4.dir/tests/hw4_tests.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/hw4.dir/tests/hw4_tests.c.o   -c /home/karin/jiarchen/hw4/tests/hw4_tests.c

CMakeFiles/hw4.dir/tests/hw4_tests.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/hw4.dir/tests/hw4_tests.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/karin/jiarchen/hw4/tests/hw4_tests.c > CMakeFiles/hw4.dir/tests/hw4_tests.c.i

CMakeFiles/hw4.dir/tests/hw4_tests.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/hw4.dir/tests/hw4_tests.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/karin/jiarchen/hw4/tests/hw4_tests.c -o CMakeFiles/hw4.dir/tests/hw4_tests.c.s

# Object files for target hw4
hw4_OBJECTS = \
"CMakeFiles/hw4.dir/src/helper.c.o" \
"CMakeFiles/hw4.dir/src/jobs.c.o" \
"CMakeFiles/hw4.dir/src/main.c.o" \
"CMakeFiles/hw4.dir/src/sf_readline.c.o" \
"CMakeFiles/hw4.dir/src/task.c.o" \
"CMakeFiles/hw4.dir/tests/hw4_tests.c.o"

# External object files for target hw4
hw4_EXTERNAL_OBJECTS = \
"/home/karin/jiarchen/hw4/lib/sf_event.o"

hw4: CMakeFiles/hw4.dir/src/helper.c.o
hw4: CMakeFiles/hw4.dir/src/jobs.c.o
hw4: CMakeFiles/hw4.dir/src/main.c.o
hw4: CMakeFiles/hw4.dir/src/sf_readline.c.o
hw4: CMakeFiles/hw4.dir/src/task.c.o
hw4: CMakeFiles/hw4.dir/tests/hw4_tests.c.o
hw4: ../lib/sf_event.o
hw4: CMakeFiles/hw4.dir/build.make
hw4: CMakeFiles/hw4.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/karin/jiarchen/hw4/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Linking C executable hw4"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/hw4.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/hw4.dir/build: hw4

.PHONY : CMakeFiles/hw4.dir/build

CMakeFiles/hw4.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/hw4.dir/cmake_clean.cmake
.PHONY : CMakeFiles/hw4.dir/clean

CMakeFiles/hw4.dir/depend:
	cd /home/karin/jiarchen/hw4/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/karin/jiarchen/hw4 /home/karin/jiarchen/hw4 /home/karin/jiarchen/hw4/cmake-build-debug /home/karin/jiarchen/hw4/cmake-build-debug /home/karin/jiarchen/hw4/cmake-build-debug/CMakeFiles/hw4.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/hw4.dir/depend

