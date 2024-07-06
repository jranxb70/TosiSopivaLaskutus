# Write information to the console.
message("Running CMakeLists.txt for app...")

set(HEADER_FILES
    
)

set(SOURCE_FILES
    main.c 
)

set (LIBS
    engine
)

# Create app.exe from main.c. Equivalent to setting
# Properties -> General -> Configuration Type to Application(.exe)
add_executable(app ${HEADER_FILES} ${SOURCE_FILES})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET app PROPERTY C_STANDARD 17)
endif()

# Add the search directory for the databaseengine library header files. This is equivalent
# to Properties -> C/C++ -> General -> Additional Include Directories.
include_directories("${PROJECT_SOURCE_DIR}/databaseengine")


# Add the databaseengine library as a dependency. Equivalent to
# Properties -> Linker -> Input -> Additional Dependencies
target_link_libraries(app PUBLIC ${LIBS})

# Create a solution folder called "executables" and add the main project (app) to it.
# (This doesn't seem to do anything in Eclipse.)

set_property(TARGET app PROPERTY FOLDER "executables")


# Adds logic to INSTALL.vcproj to copy app.exe to the install directory.
install(TARGETS app RUNTIME DESTINATION ${PROJECT_BINARY_DIR}/bin)


