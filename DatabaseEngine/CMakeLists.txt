# Write information to the console.
message("Running CMakeLists.txt for databaseengine...")

# Collect sources into CMake variables. You can use regular expressions, but CMake 
# recommends explicitly listing each source. We'll do that for the advanced subfolder.
file(GLOB ENGINE_HEADERS "engine/*.h")
set(ADDITIONAL_HEADERS "DBEngineLibraryExports.h")

set(ENGINE_SOURCES   
                    "engine/DatabaseEngine.c"
                    "engine/DatabaseUtility.c"
                    "engine/Utilities.c"
                    "engine/SqlErrorUtil.c" 
                    "engine/list.c"
 	                "engine/cJSON.c"
                    "engine/BankReferenceCalculator.c")
                    # Remove "engine/manifest.rc"

# If we just add those collected sources to the project, the folder structure will be
# lost, so we add those back explicitly. The empty stores files at the project level.
source_group("Header Files" FILES ${ENGINE_HEADERS})
source_group("Source Files" FILES ${ENGINE_SOURCES})

# Include this folder as a search directory for headers. Equivalent to
# Properties -> C/C++ -> General -> Additional Include Directories
include_directories(engine)
include_directories(${CMAKE_SOURCE_DIR}/DatabaseEngine)

# Create engine.dll with the listed sources collected in the variables. Equivalent to 
# setting Properties -> General -> Configuration Type to Dynamic Library(.dll)
add_library(engine SHARED ${ENGINE_HEADERS} ${ENGINE_SOURCES})

# Create a folder "libraries" and adds target project (engine.vcproj) under it
# (This doesn't seem to do anything in Eclipse.)
set_property(TARGET engine PROPERTY FOLDER "libraries")

# Find ODBC library
find_package(ODBC REQUIRED)

# Link the ODBC library
target_link_libraries(engine PRIVATE ${ODBC_LIBRARIES})

# Include ODBC headers
target_include_directories(engine PRIVATE ${ODBC_INCLUDE_DIRS})

# Add logic to the INSTALL project to copy engine.dll to the destination directory.
install(
	TARGETS engine
	RUNTIME DESTINATION ${PROJECT_BINARY_DIR}/bin
	LIBRARY DESTINATION ${PROJECT_BINARY_DIR}/bin
)