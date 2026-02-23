cmake_minimum_required(VERSION 3.21)
project(gbc-c-starter-kit VERSION 1.0 LANGUAGES NONE)

# === Paths ===
set(ROM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/rom")
file(MAKE_DIRECTORY "${ROM_DIR}")
set(ROM_NAME "${PROJECT_NAME}.gbc")
set(ROM_PATH "${ROM_DIR}/${ROM_NAME}")

# === Source files ===
# Explicit list preferred over glob: avoids compiling dead-code modules
# (sprite.c and background.c are unused — excluded to save ~400 bytes WRAM + ROM)
set(SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/game_system.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/game_settings.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/game_state.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gameplay.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/game_objects.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/dialogue.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/input.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/lang.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/utils.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/intro.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/splash.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/title_screen.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/options_screen.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/credits.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/game_over.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/win_screen.c"
    # sprite.c   — excluded: dead code, 352 bytes WRAM wasted
    # background.c — excluded: dead code, ~48 bytes WRAM wasted
)
file(GLOB RES_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/res/**/*.c")

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
list(APPEND LCC_FLAGS -I${CMAKE_CURRENT_SOURCE_DIR}/res/tiles)

# --- ROM banking ---
list(APPEND LCC_FLAGS -Wm-yo8)   # 8 ROM banks = 128 KB
# list(APPEND LCC_FLAGS -Wm-yo16)  # 16 ROM banks = 256 KB
# list(APPEND LCC_FLAGS -Wm-yo32)  # 32 ROM banks = 512 KB
# list(APPEND LCC_FLAGS -Wm-yo64)  # 64 ROM banks = 1 MB
# list(APPEND LCC_FLAGS -Wm-yo128) # 128 ROM banks = 2 MB
list(APPEND LCC_FLAGS -Wm-yc5)   # header: cart type = MBC5
list(APPEND LCC_FLAGS -Wl-yt5)   # linker: force cart type = 0x19 (MBC5, no RAM, no batt)

# === Build ROM ===
add_custom_command(
  OUTPUT "${ROM_PATH}"
  COMMAND "${LCC}" ${LCC_FLAGS} -o "${ROM_PATH}" ${SRC_FILES} ${RES_SOURCES}
  DEPENDS ${SRC_FILES} ${RES_SOURCES}
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
