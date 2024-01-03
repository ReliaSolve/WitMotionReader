cmake_minimum_required(VERSION 3.10)

# set the project name
project(WitMotionReader)

# Make a library from the WitMotion SDK files for Linux.
add_library(wit
	submodules/WitStandardProtocol_JY901/Linux_C/normal/serial.c 
	submodules/WitStandardProtocol_JY901/Linux_C/normal/wit_c_sdk.c 
	)
target_include_directories(wit PUBLIC submodules/WitStandardProtocol_JY901/Linux_C/normal)

# Add the example executable from the Wit-Motion SDK
add_executable(witExample submodules/WitStandardProtocol_JY901/Linux_C/normal/main.c)
target_link_libraries(witExample wit)

# Add the example to be used for development
add_executable(example example.c)
target_link_libraries(example wit)

