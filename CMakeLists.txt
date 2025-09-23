cmake_minimum_required(VERSION 3.21)
project(gbc-c-starter-kit VERSION 1.0 LANGUAGES NONE)

# === Paths ===
set(ROM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/rom")
file(MAKE_DIRECTORY "${ROM_DIR}")
set(ROM_NAME "${PROJECT_NAME}.gbc")
set(ROM_PATH "${ROM_DIR}/${ROM_NAME}")

# === Source files (.c in src/ and .c in res/sprites) ===
file(GLOB SRC_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
file(GLOB SPRITE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/res/sprites/*.c")

# === Locate tools (from GBDK_ROOT) ===
set(LCC_DEFAULT "${GBDK_ROOT}/bin/lcc.exe")
if(NOT DEFINED LCC OR LCC STREQUAL "")
  set(LCC "${LCC_DEFAULT}")
endif()
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

set(PNG2ASSET_DEFAULT "${GBDK_ROOT}/bin/png2asset.exe")
if(NOT DEFINED PNG2ASSET OR PNG2ASSET STREQUAL "")
  set(PNG2ASSET "${PNG2ASSET_DEFAULT}")
endif()
if(NOT EXISTS "${PNG2ASSET}")
  message(FATAL_ERROR
    "png2asset not found.\n"
    "  Tried PNG2ASSET='${PNG2ASSET}'\n"
    "  GBDK_ROOT='${GBDK_ROOT}'\n"
    "Fix by either:\n"
    "  1) Set env GBDK_ROOT=C:/gbdk (or your path) and reconfigure, or\n"
    "  2) Run cmake with -DGBDK_ROOT=C:/gbdk, or\n"
    "  3) Run cmake with -DPNG2ASSET=full/path/to/png2asset.exe")
endif()

# === Flags ===
set(LCC_FLAGS "")
list(APPEND LCC_FLAGS ${GBDK_CFLAGS})
list(APPEND LCC_FLAGS -I${GBDK_INCLUDE_DIR})
list(APPEND LCC_FLAGS -I${CMAKE_CURRENT_SOURCE_DIR}/include)
list(APPEND LCC_FLAGS -I${CMAKE_CURRENT_SOURCE_DIR}/res/sprites)


# === Build ROM ===
add_custom_command(
  OUTPUT "${ROM_PATH}"
  COMMAND "${LCC}" ${LCC_FLAGS} -o "${ROM_PATH}" ${SRC_FILES} ${SPRITE_SOURCES}
  DEPENDS ${SRC_FILES} ${SPRITE_SOURCES}
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  COMMENT "Building ${ROM_NAME} with ${LCC}"
  VERBATIM
)

add_custom_target(rom ALL DEPENDS "${ROM_PATH}")

# === Run in BGB ===
set(BGB_EXE "${CMAKE_CURRENT_SOURCE_DIR}/../emulator/bgbw64/bgb64.exe")
add_custom_target(run
  COMMAND "${BGB_EXE}" "${ROM_PATH}"
  DEPENDS rom
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  COMMENT "Running ${ROM_NAME} in BGB"
  VERBATIM
)

# === IntelliSense ===
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
