cmake_minimum_required(VERSION 3.21)
project(gbc-c-starter-kit VERSION 1.0 LANGUAGES NONE)

# Output ROM directory
set(ROM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/rom")
file(MAKE_DIRECTORY "${ROM_DIR}")
set(ROM_NAME "${PROJECT_NAME}.gbc")
set(ROM_PATH "${ROM_DIR}/${ROM_NAME}")

# Source files
file(GLOB SRC_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

# Direct access to lcc + includes from toolchain (or overrideable cache)
set(LCC_DEFAULT "${GBDK_ROOT}/bin/lcc.exe")
# Allow -DLCC=... to override, otherwise use default
if(NOT DEFINED LCC OR LCC STREQUAL "")
  set(LCC "${LCC_DEFAULT}")
endif()
# Validate lcc existence (final check)
if(NOT EXISTS "${LCC}")
  message(FATAL_ERROR
    "GBDK lcc not found.\n"
    "  Tried LCC='${LCC}'\n"
    "  GBDK_ROOT='${GBDK_ROOT}'\n"
    "Fix by either:\n"
    "  1) Set env GBDK_ROOT=C:/gbdk (or your path) and reconfigure, or\n"
    "  2) Run cmake with -DGBDK_ROOT=C:/gbdk, or\n"
    "  3) Run cmake with -DLCC=full/path/to/lcc.exe")
endif()

# Flags
set(LCC_FLAGS "${GBDK_CFLAGS}")
list(APPEND LCC_FLAGS "-I${GBDK_INCLUDE_DIR}")
list(APPEND LCC_FLAGS "-I${CMAKE_CURRENT_SOURCE_DIR}/include")

# Build output
add_custom_command(
  OUTPUT "${ROM_PATH}"
  COMMAND "${LCC}" ${LCC_FLAGS} -o "${ROM_PATH}" ${SRC_FILES}
  DEPENDS ${SRC_FILES}
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  COMMENT "Building ${ROM_NAME} with ${LCC}"
  VERBATIM
)
add_custom_target(rom ALL DEPENDS "${ROM_PATH}")

# Run in BGB (your path)
set(BGB_EXE "${CMAKE_CURRENT_SOURCE_DIR}/../emulator/bgbw64/bgb64.exe")
add_custom_target(run
  COMMAND "${BGB_EXE}" "${ROM_PATH}"
  DEPENDS rom
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  COMMENT "Running ${ROM_NAME} in BGB"
  VERBATIM
)

# Export compile_commands.json for IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
