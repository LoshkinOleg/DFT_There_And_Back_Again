cmake_minimum_required (VERSION 3.14)
project(FeedbackDelayNetwork)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# Ensure build dir exists.
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/build/")
	message(FATAL_ERROR "Please specify an out-of-source directory 'build/' in the project's root directory.")
endif()

# Generate FDN static lib.
file(GLOB_RECURSE FDN_include ${PROJECT_SOURCE_DIR}/FDN/include/*.h)
file(GLOB_RECURSE FDN_src ${PROJECT_SOURCE_DIR}/FDN/src/*.cpp)
add_library(FDN STATIC ${FDN_include} ${FDN_src})
set_target_properties(FDN PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(FDN PUBLIC
	${PROJECT_SOURCE_DIR}/FDN/include/
	${PROJECT_SOURCE_DIR}/thirdparty/portaudio/include/
	${PROJECT_SOURCE_DIR}/thirdparty/easy_profiler/include/
	${PROJECT_SOURCE_DIR}/thirdparty/dr_wav/include/
	${PROJECT_SOURCE_DIR}/thirdparty/sdl/include/
	${PROJECT_SOURCE_DIR}/thirdparty/spdlog/include/
	)
target_link_libraries(FDN PRIVATE
	general ${PROJECT_SOURCE_DIR}/thirdparty/portaudio/lib/portaudio_static_x64.lib
	general ${PROJECT_SOURCE_DIR}/thirdparty/easy_profiler/lib/easy_profiler.lib
	general ${PROJECT_SOURCE_DIR}/thirdparty/sdl/lib/SDL2.lib
	general ${PROJECT_SOURCE_DIR}/thirdparty/sdl/lib/SDL2main.lib
	general ${PROJECT_SOURCE_DIR}/thirdparty/sdl/lib/SDL2test.lib
	optimized ${PROJECT_SOURCE_DIR}/thirdparty/spdlog/lib/spdlog.lib
	debug ${PROJECT_SOURCE_DIR}/thirdparty/spdlog/lib/spdlogd.lib
	)
set_target_properties(FDN PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/FDN/lib"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/FDN/lib"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/FDN/bin"
)

# Generate app executable.
file(GLOB_RECURSE Application_include ${PROJECT_SOURCE_DIR}/Application/include/*.h)
file(GLOB_RECURSE Application_src ${PROJECT_SOURCE_DIR}/Application/src/*.cpp)
add_executable(Application ${Application_include} ${Application_src})
target_include_directories(Application PUBLIC
	${PROJECT_SOURCE_DIR}/Application/include/
	${PROJECT_SOURCE_DIR}/FDN/include/
	)
target_link_libraries(Application PRIVATE
	general FDN
	)
set_target_properties(Application PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/Application/lib"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/Application/lib"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/Application/bin"
)

# Create folder for holding easy_profiler's profiling data.
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/build/profilingData)