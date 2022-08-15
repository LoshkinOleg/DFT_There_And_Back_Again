# The Frequency Domain Visualization

A repo containing the code used in the "The Frequency Domain: Or, There and Back Again" blogpost.
Clone the repo, generate a VS solution with CMake with an out-of-source build located in a "/build" directory of the repo.
Set the Application project as the default project, compile and execute moveDlls.bat to launch the visualization.
Modify "/Application/include/ExerciseVisualization.h" to implement your own DFT and IDFT and in "/Application/src/main.cpp" comment / uncomment the right header.

Warning: if you enable profiling with easy_profiler in CMake's GUI, be careful not to let the application run for too long: the application's update loop does not sleep so the profiler output quickly becomes huge (half a GB in a few seconds only)!