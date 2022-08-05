cmake_minimum_required (VERSION 3.14) # Define minimal CMake version.
project(MyDFT_project) # Name the visual studio solution.

# Set the C++20 as the C++ standard to use (ISO/IEC 14882).
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# Ensure build dir exists. We'll be relying on this for finding relative paths in the code and any external tools.
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/build/")
	message(FATAL_ERROR "Please specify an out-of-source directory 'build/' in the project's root directory. If you don't know what an out-of-source build is, here's a link: https://cmake.org/cmake/help/book/mastering-cmake/chapter/Getting%20Started.html")
endif()

# Retrieve source and interface files for MyUtils library.
file(GLOB_RECURSE MyUtils_include ${PROJECT_SOURCE_DIR}/MyUtils/include/*.h) # file(<keyword> <variable> <path>) function retrieves the file (or files) matching <path> and stores it as a string in <variable>. The GLOBAL_RECURSE <keyword> specifies that subfolders of <path> should also be scanned.
file(GLOB_RECURSE MyUtils_src ${PROJECT_SOURCE_DIR}/MyUtils/src/*.cpp)

# Define MyUtils static library. It'll hold some miscealeous code. Static means that the compiled .lib file gets incorporated in the Application.exe .
add_library(MyUtils STATIC ${MyUtils_include} ${MyUtils_src}) # add_library(<name> <keyword> <files>) defines a new library build target. STATIC as <keyword> makes the compiler generate a static library (a .lib file).
target_include_directories(MyUtils # target_include_directories(<name> <keyword> <path>) defines directories that contain interfaces to thirdparty code.
	PRIVATE ${PROJECT_SOURCE_DIR}/MyUtils/include/ # Include MyUtils interface to be able to use it. The PRIVATE as <keyword> makes the directories specified in <path> visible only to MyUtils. Good explanation of PUBLIC, PRIVATE and INTERFACE here: https://stackoverflow.com/questions/26243169/cmake-target-include-directories-meaning-of-scope
	)
# Define output paths for the compiler.
# TODO: this doesn't work?
set_target_properties(MyUtils PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/MyUtils/lib") # set_target_properties(<name> PROPERTIES LIBRARY_OUTPUT_DIRECTORY <path>) defines where to put generated .lib files for a static library build.

# Retrieve source and interface files for MyDFT library.
file(GLOB_RECURSE MyDFT_include ${PROJECT_SOURCE_DIR}/MyDFT/include/*.h)
file(GLOB_RECURSE MyDFT_src ${PROJECT_SOURCE_DIR}/MyDFT/src/*.cpp)

# Define MyDFT static library. It'll hold generic DFT code.
add_library(MyDFT STATIC ${MyDFT_include} ${MyDFT_src})
# Adding MyFDT's dependancies.
target_include_directories(MyDFT 
	PRIVATE ${PROJECT_SOURCE_DIR}/MyDFT/include/
	PRIVATE ${PROJECT_SOURCE_DIR}/thirdparty/Simple-FFT/include/ # Simple-FFT is a header-only FFT and IFFT library. Not as optimized as other more popular libraries but can be easily inspected, is simple and is easy to add to a project. The PRIVATE as <keyword> makes the directories specified in <path> visible only to MyDFT. This is what we want since we don't want anything that depends on MyDFT to be aware of implementation details (Simple-FFT is only used in MyDFT.cpp).
	PRIVATE ${PROJECT_SOURCE_DIR}/MyUtils/include/ # Include MyUtils interface to be able to use it.
	)
target_link_libraries(MyDFT PRIVATE # target_link_libraries(<target> <keyword> <build type> <other target or .libs>) links <target> with it's dependancies. <build type> can either be "general", "debug" and "optimized" and can be used to specify different libs for different build types. PRIVATE is used here since only the implementation of MyDFT will depend on MyUtils, not the interface.
	general MyUtils # Linking with MyUtils with "general" prefix since we only defined one type of MyDFT target anyways.
	)
# Define output paths for the compiler.
set_target_properties(MyDFT PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/MyDFT/lib")

# Define Application executable.
file(GLOB_RECURSE Application_include ${PROJECT_SOURCE_DIR}/Application/include/*.h) # Retrieve source and interface files for the Application target.
file(GLOB_RECURSE Application_src ${PROJECT_SOURCE_DIR}/Application/src/*.cpp)
add_executable(Application ${Application_include} ${Application_src}) # add_executable(<name> <files>) defines the Application executable build target.

# Add imgui source and interface files to Application target since the library is meant to be part of a user's source code. imgui is a lightweight graphical user interface library.
file(GLOB_RECURSE thirdparty_imgui_include ${PROJECT_SOURCE_DIR}/thirdparty/imgui/include/*.h) # Retrieve source and interface files for imgui.
file(GLOB_RECURSE thirdparty_imgui_src ${PROJECT_SOURCE_DIR}/thirdparty/imgui/src/*.cpp)
target_sources(Application PRIVATE ${thirdparty_imgui_include} ${thirdparty_imgui_src}) # target_sources(<target> <keyword> <files>) adds the interface and source files of imgui as source files for Application. We're specifying PRIVATE as <keyword> since nothing will depend on an executable target.
source_group("thirdparty" FILES ${thirdparty_imgui_include} ${thirdparty_imgui_src}) # source_group(<name> <keyword> <path>) defines a new visual studio filter (little folders in the solution view). For tidiness' sake, we're placing the source and interface files in a separate visual studio filter to avoid having imgui files in the same place as our own.

# Add the rest of dependancies of Application.
target_include_directories(Application PRIVATE # Privately add external interface directories.
	${PROJECT_SOURCE_DIR}/MyUtils/include/ # Our own utility static library.
	${PROJECT_SOURCE_DIR}/MyDFT/include/ # Our own DFT static library.
	${PROJECT_SOURCE_DIR}/Application/include/ # Target's own interface.
	${PROJECT_SOURCE_DIR}/thirdparty/imgui/include/ # Imgui interface.
	${PROJECT_SOURCE_DIR}/thirdparty/dr_wav/include/ # Header-only library for reading and writing .wav files.
	${PROJECT_SOURCE_DIR}/thirdparty/easy_profiler/include/ # Open-source and easy to add to a project profiling library.
	${PROJECT_SOURCE_DIR}/thirdparty/portaudio/include/ # Open-source cross-platform library for interacting with audio devices.
	${PROJECT_SOURCE_DIR}/thirdparty/sdl/include/ # Open-source cross-platform library for developing PC applications, popular for C++ game development. Can interface with graphics API like OpenGL or Vulkan with some extra set-up if needed.
	)
target_link_libraries(Application PRIVATE # PRIVATE is used as <keyword> here since Application is an executable, nothing will depend on it.
	general MyDFT # Linking with MyDFT with "general" prefix since we only defined one type of MyDFT target anyways.
	general MyUtils # Linking with MyUtils with "general" prefix since we only defined one type of MyUtils target anyways.
	general ${PROJECT_SOURCE_DIR}/thirdparty/easy_profiler/lib/easy_profiler.lib # Linking against the .lib of the easy_profiler dynamic library. This means that the pre-compiled "easy_profiler.dll" will need to be placed besides the Application.exe .
	general ${PROJECT_SOURCE_DIR}/thirdparty/portaudio/lib/portaudio_static_x64.lib # Linking against the portaudio static library.
	general ${PROJECT_SOURCE_DIR}/thirdparty/sdl/lib/SDL2.lib # Linking against the .lib of sdl's dynamic library. This means that the pre-compiled "SDL2.dll" will need to be placed besides the Application.exe .
	)

# Put Application's executable in a specific location.
set_target_properties(Application PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/Application/bin")

# Make some folders we might need.
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/profilerOutputs) # Folder for holding easy_profiler's profiling data. file(MAKE_DIRECTORY <dir>) creates a new specified directiory if it doesn't exist yet.
add_compile_definitions(APPLICATION_PROFILER_OUTPUTS_DIR="${PROJECT_SOURCE_DIR}/profilerOutputs/") # Add a define to easily write the profiler's output directory path in the code. add_compile_definitions(<define>) adds a solution-wide preprocessor definition to be used in the code.
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/wavOutputs) # Folder for holding wav file outputs that Application generates.
add_compile_definitions(APPLICATION_WAV_OUTPUTS_DIR="${PROJECT_SOURCE_DIR}/wavOutputs/") # Add a define to easily write the wav output directory path in the code.
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/txtOutputs) # Folder for holding txt file outputs that Application generates.
add_compile_definitions(APPLICATION_TXT_OUTPUTS_DIR="${PROJECT_SOURCE_DIR}/txtOutputs/") # Add a define to easily write the text output directory path in the code.

# Defining some options for the user to configure the desired behaviour of the generated programs.
set(USE_EASY_PROFILER OFF CACHE BOOL "Whether to enable profiling with easy_profiler. Generated .prof files will be located under /profilerOutputs/") # set(<define> <default value> CACHE <variable type> <description>) creates a variable interactible in the CMake GUI.
if (USE_EASY_PROFILER) # If the use of easy_profiler is desired, add a global preprocessor definition.
	add_compile_definitions(BUILD_WITH_EASY_PROFILER) # BUILD_WITH_EASY_PROFILER is the define that the library's user must declare when they wish to use easy_profiler.
endif()