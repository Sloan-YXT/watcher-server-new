# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/yaoxuetao/CMPW-server

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/yaoxuetao/CMPW-server/build

# Include any dependencies generated for this target.
include CMakeFiles/server.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/server.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/server.dir/flags.make

CMakeFiles/server.dir/server.cpp.o: CMakeFiles/server.dir/flags.make
CMakeFiles/server.dir/server.cpp.o: ../server.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/yaoxuetao/CMPW-server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/server.dir/server.cpp.o"
	/usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/server.dir/server.cpp.o -c /home/yaoxuetao/CMPW-server/server.cpp

CMakeFiles/server.dir/server.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/server.dir/server.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/yaoxuetao/CMPW-server/server.cpp > CMakeFiles/server.dir/server.cpp.i

CMakeFiles/server.dir/server.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/server.dir/server.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/yaoxuetao/CMPW-server/server.cpp -o CMakeFiles/server.dir/server.cpp.s

CMakeFiles/server.dir/server.cpp.o.requires:

.PHONY : CMakeFiles/server.dir/server.cpp.o.requires

CMakeFiles/server.dir/server.cpp.o.provides: CMakeFiles/server.dir/server.cpp.o.requires
	$(MAKE) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/server.cpp.o.provides.build
.PHONY : CMakeFiles/server.dir/server.cpp.o.provides

CMakeFiles/server.dir/server.cpp.o.provides.build: CMakeFiles/server.dir/server.cpp.o


CMakeFiles/server.dir/database.cpp.o: CMakeFiles/server.dir/flags.make
CMakeFiles/server.dir/database.cpp.o: ../database.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/yaoxuetao/CMPW-server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/server.dir/database.cpp.o"
	/usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/server.dir/database.cpp.o -c /home/yaoxuetao/CMPW-server/database.cpp

CMakeFiles/server.dir/database.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/server.dir/database.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/yaoxuetao/CMPW-server/database.cpp > CMakeFiles/server.dir/database.cpp.i

CMakeFiles/server.dir/database.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/server.dir/database.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/yaoxuetao/CMPW-server/database.cpp -o CMakeFiles/server.dir/database.cpp.s

CMakeFiles/server.dir/database.cpp.o.requires:

.PHONY : CMakeFiles/server.dir/database.cpp.o.requires

CMakeFiles/server.dir/database.cpp.o.provides: CMakeFiles/server.dir/database.cpp.o.requires
	$(MAKE) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/database.cpp.o.provides.build
.PHONY : CMakeFiles/server.dir/database.cpp.o.provides

CMakeFiles/server.dir/database.cpp.o.provides.build: CMakeFiles/server.dir/database.cpp.o


CMakeFiles/server.dir/cutil.cpp.o: CMakeFiles/server.dir/flags.make
CMakeFiles/server.dir/cutil.cpp.o: ../cutil.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/yaoxuetao/CMPW-server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/server.dir/cutil.cpp.o"
	/usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/server.dir/cutil.cpp.o -c /home/yaoxuetao/CMPW-server/cutil.cpp

CMakeFiles/server.dir/cutil.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/server.dir/cutil.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/yaoxuetao/CMPW-server/cutil.cpp > CMakeFiles/server.dir/cutil.cpp.i

CMakeFiles/server.dir/cutil.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/server.dir/cutil.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/yaoxuetao/CMPW-server/cutil.cpp -o CMakeFiles/server.dir/cutil.cpp.s

CMakeFiles/server.dir/cutil.cpp.o.requires:

.PHONY : CMakeFiles/server.dir/cutil.cpp.o.requires

CMakeFiles/server.dir/cutil.cpp.o.provides: CMakeFiles/server.dir/cutil.cpp.o.requires
	$(MAKE) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/cutil.cpp.o.provides.build
.PHONY : CMakeFiles/server.dir/cutil.cpp.o.provides

CMakeFiles/server.dir/cutil.cpp.o.provides.build: CMakeFiles/server.dir/cutil.cpp.o


CMakeFiles/server.dir/faceDetect.cpp.o: CMakeFiles/server.dir/flags.make
CMakeFiles/server.dir/faceDetect.cpp.o: ../faceDetect.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/yaoxuetao/CMPW-server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/server.dir/faceDetect.cpp.o"
	/usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/server.dir/faceDetect.cpp.o -c /home/yaoxuetao/CMPW-server/faceDetect.cpp

CMakeFiles/server.dir/faceDetect.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/server.dir/faceDetect.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/yaoxuetao/CMPW-server/faceDetect.cpp > CMakeFiles/server.dir/faceDetect.cpp.i

CMakeFiles/server.dir/faceDetect.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/server.dir/faceDetect.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/yaoxuetao/CMPW-server/faceDetect.cpp -o CMakeFiles/server.dir/faceDetect.cpp.s

CMakeFiles/server.dir/faceDetect.cpp.o.requires:

.PHONY : CMakeFiles/server.dir/faceDetect.cpp.o.requires

CMakeFiles/server.dir/faceDetect.cpp.o.provides: CMakeFiles/server.dir/faceDetect.cpp.o.requires
	$(MAKE) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/faceDetect.cpp.o.provides.build
.PHONY : CMakeFiles/server.dir/faceDetect.cpp.o.provides

CMakeFiles/server.dir/faceDetect.cpp.o.provides.build: CMakeFiles/server.dir/faceDetect.cpp.o


# Object files for target server
server_OBJECTS = \
"CMakeFiles/server.dir/server.cpp.o" \
"CMakeFiles/server.dir/database.cpp.o" \
"CMakeFiles/server.dir/cutil.cpp.o" \
"CMakeFiles/server.dir/faceDetect.cpp.o"

# External object files for target server
server_EXTERNAL_OBJECTS =

server: CMakeFiles/server.dir/server.cpp.o
server: CMakeFiles/server.dir/database.cpp.o
server: CMakeFiles/server.dir/cutil.cpp.o
server: CMakeFiles/server.dir/faceDetect.cpp.o
server: CMakeFiles/server.dir/build.make
server: /usr/local/lib/libopencv_gapi.so.4.5.5
server: /usr/local/lib/libopencv_highgui.so.4.5.5
server: /usr/local/lib/libopencv_ml.so.4.5.5
server: /usr/local/lib/libopencv_objdetect.so.4.5.5
server: /usr/local/lib/libopencv_photo.so.4.5.5
server: /usr/local/lib/libopencv_stitching.so.4.5.5
server: /usr/local/lib/libopencv_video.so.4.5.5
server: /usr/local/lib/libopencv_videoio.so.4.5.5
server: /usr/local/lib/libopencv_imgcodecs.so.4.5.5
server: /usr/local/lib/libopencv_dnn.so.4.5.5
server: /usr/local/lib/libopencv_calib3d.so.4.5.5
server: /usr/local/lib/libopencv_features2d.so.4.5.5
server: /usr/local/lib/libopencv_flann.so.4.5.5
server: /usr/local/lib/libopencv_imgproc.so.4.5.5
server: /usr/local/lib/libopencv_core.so.4.5.5
server: CMakeFiles/server.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/yaoxuetao/CMPW-server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking CXX executable server"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/server.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/server.dir/build: server

.PHONY : CMakeFiles/server.dir/build

CMakeFiles/server.dir/requires: CMakeFiles/server.dir/server.cpp.o.requires
CMakeFiles/server.dir/requires: CMakeFiles/server.dir/database.cpp.o.requires
CMakeFiles/server.dir/requires: CMakeFiles/server.dir/cutil.cpp.o.requires
CMakeFiles/server.dir/requires: CMakeFiles/server.dir/faceDetect.cpp.o.requires

.PHONY : CMakeFiles/server.dir/requires

CMakeFiles/server.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/server.dir/cmake_clean.cmake
.PHONY : CMakeFiles/server.dir/clean

CMakeFiles/server.dir/depend:
	cd /home/yaoxuetao/CMPW-server/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/yaoxuetao/CMPW-server /home/yaoxuetao/CMPW-server /home/yaoxuetao/CMPW-server/build /home/yaoxuetao/CMPW-server/build /home/yaoxuetao/CMPW-server/build/CMakeFiles/server.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/server.dir/depend

