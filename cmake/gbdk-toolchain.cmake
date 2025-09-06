# Cross
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR gbz80)
# Evita try-run
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# 1) GBDK_ROOT con default + cache
if(NOT DEFINED GBDK_ROOT OR GBDK_ROOT STREQUAL "")
  if(DEFINED ENV{GBDK_ROOT} AND NOT "$ENV{GBDK_ROOT}" STREQUAL "")
    set(GBDK_ROOT "$ENV{GBDK_ROOT}")
  else()
    set(GBDK_ROOT "C:/gbdk")
  endif()
endif()
set(GBDK_ROOT "${GBDK_ROOT}" CACHE PATH "Path to GBDK-2020 root" FORCE)

# 2) Include dir + flags in cache (utile per IntelliSense)
set(GBDK_INCLUDE_DIR "${GBDK_ROOT}/include" CACHE PATH "GBDK include dir" FORCE)
set(GBDK_CFLAGS "-O2" CACHE STRING "Default GBDK lcc compile flags" FORCE)

# 3) Compilatori: mettili in CACHE per soddisfare CMake Tools
set(CMAKE_C_COMPILER   "${GBDK_ROOT}/bin/lcc.exe" CACHE FILEPATH "GBDK lcc (C compiler driver)" FORCE)
# Alcuni frontend VB/IntelliSense chiedono anche C++
set(CMAKE_CXX_COMPILER "${GBDK_ROOT}/bin/lcc.exe" CACHE FILEPATH "GBDK lcc (used also for C++)" FORCE)

# Suggerisci a CMake che i compilatori sono “forzati” e non vanno cambiati
set(CMAKE_C_COMPILER_FORCED   TRUE CACHE BOOL "" FORCE)
set(CMAKE_CXX_COMPILER_FORCED TRUE CACHE BOOL "" FORCE)

# 4) Evita test di funzionalità che potrebbero fallire con lcc
set(CMAKE_C_STANDARD          99 CACHE STRING "" FORCE)
set(CMAKE_C_EXTENSIONS        OFF CACHE BOOL "" FORCE)
# Nota: NON abilitiamo LANGUAGES nel project root; useremo custom commands con lcc.

# 5) Avviso non-bloccante se lcc manca
if(NOT EXISTS "${CMAKE_C_COMPILER}")
  message(STATUS "Warning: lcc not found at ${CMAKE_C_COMPILER}. Will validate in root CMakeLists.")
endif()
