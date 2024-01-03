cmake_minimum_required(VERSION 3.10)

# set the project name
project(WitMotionReader)

# Make a library from the WitMotion files.
add_library(wit
	submodules/WitStandardProtocol_JY901/Linux_C/normal/serial.c 
	submodules/WitStandardProtocol_JY901/Linux_C/normal/wit_c_sdk.c 
	)
target_include_directories(wit PUBLIC submodules/WitStandardProtocol_JY901/Linux_C/normal)

# add the executable
add_executable(witExample submodules/WitStandardProtocol_JY901/Linux_C main.cpp)
target_link_libraries(witExample wit)

