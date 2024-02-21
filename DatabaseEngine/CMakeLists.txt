# Write information to the console.
message("Running CMakeLists.txt for databaseengine...")

# Collect sources into CMake variables. You can use regular expressions, but CMake 
# recommends explicitly listing each source. We'll do that for the advanced subfolder.
file(GLOB ENGINE_HEADERS "*.h")
set(ENGINE_SOURCES  "engine/DatabaseEngine.h" 
                    "engine/DatabaseEngine.c" 
                    "engine/Utilities.h" 
                    "engine/Utilities.c" 
                    "engine/SqlErrorUtil.h" 
                    "engine/SqlErrorUtil.c" 
                    "engine/list.h" 
                    "engine/list.c"
		    "engine/cJSON.h"
		    "engine/cJSON.c")


# If we just add those collected sources to the project, the folder structure will be
# lost, so we add those back explicitly. The empty stores files at the project level.
source_group("" FILES ${ENGINE_HEADERS})
source_group("engine" FILES ${ENGINE_SOURCES})

# Include this folder as a search directory for headers. Equivalent to
# Properties -> C/C++ -> General -> Additional Include Directories
include_directories(.)

#if (MSVC)
#  set(CMAKE_CXX_FLAGS_DEBUG "/ZI")
#  set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "/INCREMENTAL")
#  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/INCREMENTAL")
#endif()



# set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /ZI")

# Create engine.dll with the listed sources collected in the variables. Equivalent to 
# setting Properties -> General -> Configuration Type to Dynamic Library(.dll)
add_library(engine SHARED ${ENGINE_HEADERS} ${ENGINE_SOURCES})
# Create a folder "libraries" and adds target project (engine.vcproj) under it
# (This doesn't seem to do anything in Eclipse.)
set_property(TARGET engine PROPERTY FOLDER "libraries")

# if (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)

    # option 1: put the following command before the creation of the first
    #           compiled target you want to apply the option to;
    #           this applies to targets created in this dir and subdirectories
    #add_link_options("/INCREMENTAL")
    #add_compile_option($<$<CONFIG:Debug>:/ZI>)

    # option 2: apply the option on a per-target basis
    # target_link_options(engine PRIVATE "/INCREMENTAL")
    # target_compile_options(engine PRIVATE "/ZI")

# endif()

# Find ODBC library
find_package(ODBC REQUIRED)

# Link the ODBC library
target_link_libraries(engine PRIVATE ${ODBC_LIBRARIES})

# Include ODBC headers
target_include_directories(engine PRIVATE ${ODBC_INCLUDE_DIRS})

# set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /ZI")

# Add logic to the INSTALL project to copy engine.dll to the destination directory.
install(
	TARGETS engine
	RUNTIME DESTINATION ${PROJECT_BINARY_DIR}/bin
	LIBRARY DESTINATION ${PROJECT_BINARY_DIR}/bin
)
