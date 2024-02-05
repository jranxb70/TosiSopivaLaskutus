cmake_minimum_required (VERSION 3.27)

# Write information to the console.
message("Running CMakeLists.txt for TosiSopiva...")

# Maps to a solution file (TosiSopiva.sln). The solution will 
# have all targets (exe, lib, dll) as projects (.vcproj)
project(TosiSopiva)

# Turn on the ability to create folders to organize projects.
# By default, it creates the "CMakePredefinedTargets" folder to 
# store the ALL_BUILD, INSTALL, RUN_TESTS, and ZERO_CHECK projects.
# (This doesn't seem to do anything in Eclipse.)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# You can set compiler flags like this.
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")


# Add sub-directories where more CMakeLists.txt exist.
add_subdirectory(app)
add_subdirectory(databaseengine)

# Turn on CMake testing capabilities
enable_testing()
add_test(AppTest ${PROJECT_BINARY_DIR}/bin/app.exe connectionstring.txt)

