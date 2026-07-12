cmake_minimum_required(VERSION 3.25...4.2 FATAL_ERROR)

# MSVC runtime library flags are selected by an abstraction
cmake_policy(SET CMP0091 NEW)

project(tony2 C CXX)

if (NOT WIN32)
  message(FATAL_ERROR "Windows is the only supported platform (for now...)")
endif()

include(CheckCXXSourceCompiles)
include(CMakeDependentOption)
include(CMakePushCheckState)
include("${CMAKE_CURRENT_LIST_DIR}/cmake/reccmp.cmake")

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
option(ENABLE_CLANG_TIDY "Enable clang-tidy")
if (ENABLE_CLANG_TIDY)
  find_program(CLANG_TIDY_BIN NAMES "clang-tidy")
  set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY_BIN}")
  set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_BIN}")
endif()

math(EXPR bits "8 * ${CMAKE_SIZEOF_VOID_P}")
message(STATUS "Building ${bits}-bit Tony & Friends - New Adventures")
if (NOT bits EQUAL 32)
  message(WARNING "Only 32-bit executables are supported")
endif()

cmake_dependent_option(TONY2_USE_DX6 "Build with internal DirectX 6 SDK" ON "CMAKE_SIZEOF_VOID_P EQUAL 4" OFF)
if (TONY2_USE_DX6)
  include_directories("${PROJECT_SOURCE_DIR}/3rdparty/dx6/inc")
  link_directories("${PROJECT_SOURCE_DIR}/3rdparty/dx6/lib")
  foreach(name d3dim d3drm d3dxof ddraw ddraw16 dinput dplayx dsetup dsound dxguid)
    add_library(${name} IMPORTED UNKNOWN)
    set_property(TARGET ${name} PROPERTY IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/3rdparty/dx6/lib/${name}.lib")
  endforeach()
endif()

set(MSVC_FOR_DECOMP FALSE)
if (MSVC)
  # Visual C++ 6.0 -> cl version 12.x
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "13.0")
    set(MSVC_FOR_DECOMP TRUE)
  endif()
endif()

function(add_cxx_warning WARNING)
  if (TONY2_WERROR)
    set(compiler_option "-Werror=${WARNING}")
  else()
    set(compiler_option "-W${WARNING}")
  endif()
  string(MAKE_C_IDENTIFIER "COMPILER_SUPPORTS${compiler_option}" varname)

  cmake_push_check_state(RESET)
  set(CMAKE_REQUIRED_FLAGS "${compiler_option} ")
  if (MSVC)
    string(APPEND CMAKE_REQUIRED_FLAGS "/WX")
  else()
    string(APPEND CMAKE_REQUIRED_FLAGS "-Werror")
  endif()
  check_cxx_source_compiles("int main() { return 0; }" ${varname})
  cmake_pop_check_state()

  if (${varname})
    add_compile_options(${compiler_option})
  endif()
endfunction()

message(STATUS "MSVC for decompilation: ${MSVC_FOR_DECOMP}")

option(TONY2_WERROR "Treat warnings as errors" OFF)
option(TONY2_DECOMP_ASSERT "Assert struct size" ${MSVC_FOR_DECOMP})

add_cxx_warning(parentheses)

find_program(IWYU_BIN NAMES "include-what-you-use")
if(IWYU_BIN)
  set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "${IWYU_BIN}" -target i686-pc-windows-msvc "-Xiwyu" "--mapping_file=${PROJECT_SOURCE_DIR}/tools/iwyu.imp")
endif()

# --- TONY2.EXE target ---
add_executable(tony2 WIN32
  # Ordered by the original TONY2.EXE link order (median address per object).
  TONY2/src/objects/gameobject.cpp
  TONY2/src/io/inputmanager.cpp
  TONY2/src/video/videomanager.cpp
  TONY2/src/video/blit.cpp
  TONY2/src/objects/objectmanager.cpp
  TONY2/src/game/gamemanager.cpp
  TONY2/src/game/main.cpp
  TONY2/src/game/camera.cpp
  TONY2/src/engine.cpp
  TONY2/src/sound/soundglue.cpp
  TONY2/src/io/langfile.cpp
  util/decomp.cpp
)
if(MSVC)
  target_link_options(tony2 PRIVATE "-MAP:$<TARGET_FILE_DIR:tony2>/$<TARGET_FILE_BASE_NAME:tony2>.map")
endif()
target_compile_definitions(tony2 PRIVATE BUILDING_TONY2)
target_compile_definitions(tony2 PRIVATE DIRECTINPUT_VERSION=0x500)
reccmp_add_target(tony2 ID TONY2)

target_include_directories(tony2 PRIVATE
  "${PROJECT_SOURCE_DIR}/TONY2/include"
  "${PROJECT_SOURCE_DIR}/TONY2"
  "${PROJECT_SOURCE_DIR}/util"
)

# --- VC6-vintage sound library TU ---
# The third-party sound library in TONY2.EXE was compiled with VC6 RTM (cl 12.00.8168)
# at /Od while the game code is VC5. Point TONY2_VC6_DIR at a portable MSVC 6.0 RTM
# toolchain (https://github.com/archaic-msvc/msvc600) to compile these TUs with the
# correct vintage; otherwise they are built with the main compiler (still links, but
# the sound-library functions will not match).
#
# Two invocation paths share the same cl flags: tools/cl_vc6.sh drives the compiler
# under Wine (a toolchain checkout carrying an activate_x86 wrapper, as used locally on
# macOS), and tools/cl_vc6.bat drives it on native Windows via the toolchain's own
# VC98/Bin/VCVARS32.BAT (as used in CI).
set(TONY2_VC6_DIR "" CACHE PATH "MSVC 6.0 RTM toolchain directory for the sound-library TU")
set(tony2_vc6_tus TONY2/src/sound/soundlib.c TONY2/src/sound/soundstream.c TONY2/src/sound/soundwave.c
                  TONY2/src/sound/soundds.c
)
set(tony2_vc6_wrapper "")
if(TONY2_VC6_DIR AND EXISTS "${TONY2_VC6_DIR}/activate_x86")
  set(tony2_vc6_wrapper bash "${CMAKE_CURRENT_SOURCE_DIR}/tools/cl_vc6.sh" "${TONY2_VC6_DIR}")
elseif(TONY2_VC6_DIR AND EXISTS "${TONY2_VC6_DIR}/VC98/Bin/VCVARS32.BAT")
  set(tony2_vc6_wrapper cmd /c "${CMAKE_CURRENT_SOURCE_DIR}/tools/cl_vc6.bat" "${TONY2_VC6_DIR}")
endif()
if(tony2_vc6_wrapper)
  foreach(vc6_tu ${tony2_vc6_tus})
    get_filename_component(vc6_tu_name "${vc6_tu}" NAME_WE)
    set(vc6_tu_obj "${CMAKE_CURRENT_BINARY_DIR}/${vc6_tu_name}.obj")
    add_custom_command(
      OUTPUT "${vc6_tu_obj}"
      COMMAND ${tony2_vc6_wrapper}
              /nologo /c /MT /Od /Z7 /W3 /DWIN32 /D_WINDOWS /DNDEBUG
              "/I${PROJECT_SOURCE_DIR}/TONY2/include" "/I${PROJECT_SOURCE_DIR}/util"
              "/Fo${vc6_tu_obj}"
              "${CMAKE_CURRENT_SOURCE_DIR}/${vc6_tu}"
      DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${vc6_tu}"
      COMMENT "Compiling ${vc6_tu_name}.c with VC6 RTM (/Od)"
      VERBATIM)
    target_sources(tony2 PRIVATE "${vc6_tu_obj}")
  endforeach()
else()
  # Compiled with the main C compiler. This third-party sound library is C with inline x86 assembly
  # and naked functions that clang-tidy's parser rejects, so skip linting on these TUs (cl still
  # compiles them). SKIP_LINTING is ignored by CMake < 3.27, which does not run the linter here.
  set_source_files_properties(${tony2_vc6_tus} PROPERTIES SKIP_LINTING ON)
  target_sources(tony2 PRIVATE ${tony2_vc6_tus})
endif()

target_link_libraries(tony2 PRIVATE
  nafxcw
  dxguid
  ddraw
  dinput
  dsound
  ole32
  winmm
  shlwapi
  comdlg32
)

set_property(TARGET tony2 PROPERTY OUTPUT_NAME "TONY2")
set_property(TARGET tony2 PROPERTY SUFFIX ".EXE")

add_custom_target(game DEPENDS tony2)

# --- Modern compiler compatibility ---
# Scoped to the target (not add_compile_options/add_definitions) so they apply even though tony2 is
# defined earlier; /Zc:__cplusplus in particular is required for <afx.h>/the STL to see a C++11
# __cplusplus and not reject compat.h's override/static_assert fallbacks.
if (MSVC)
  if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "15")
    target_compile_definitions(tony2 PRIVATE _CRT_SECURE_NO_WARNINGS)
  endif()
  # Visual Studio 2017 version 15.7 needs "/Zc:__cplusplus" for __cplusplus
  if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.14.26428")
    target_compile_options(tony2 PRIVATE -Zc:__cplusplus)
  endif()
endif()

if (TONY2_DECOMP_ASSERT)
  message(STATUS "Decomp asserts enabled")
  set_property(TARGET tony2 APPEND PROPERTY COMPILE_DEFINITIONS "ENABLE_DECOMP_ASSERTS")
endif()

if (MSVC_FOR_DECOMP)
  # These flags have been taken from the defaults for a Visual C++ 5.0 project (the compiler the
  # game was originally built with; the GSM codec library TUs use VC6 RTM) and tweaked slightly
  # to produce more debugging info for reccmp. They ensure a recompilation that can be
  # byte/instruction accurate to the original binary.
  #
  # /Gy- because the original TONY2.EXE was compiled without function-level linking: functions
  # inside a TU are padded with compiler-emitted 0x90 NOPs, and no COMDAT folding occurs anywhere
  # in the binary. (VC6's /O2 would otherwise imply /Gy.) This also keeps /OPT:REF from stripping
  # matched-but-not-yet-referenced functions while the WinMain call graph is still undecompiled.
  if (TARGET tony2)
    target_link_options(tony2 PRIVATE "/OPT:REF,ICF")
  endif()

  # TONY2.EXE has its runtime library statically linked (/MT)
  set_property(TARGET tony2 PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

  # Embed debug information (/Z7) for Debug and RelWithDebInfo configuration type
  set_property(TARGET tony2 PROPERTY MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>")

  # The original was compiled without /Gy (see the /OPT note above), but VC5/VC6 cl does not
  # accept /Gy- to negate /O2's implied /Gy, so /O2 is spelled out as its documented expansion
  # (/O2 = /Ob1 /Og /Oi /Ot /Oy /Gs /Gf /Gy) minus /Gy.
  set(CMAKE_CXX_FLAGS "/W3 /GX /DWIN32 /D_WINDOWS")
  set(CMAKE_CXX_FLAGS_DEBUG "/Gm /Od /D_DEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE "/Ob1 /Og /Oi /Ot /Oy /Gs /Gf /DNDEBUG")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/Ob1 /Og /Oi /Ot /Oy /Gs /Gf /DNDEBUG")
  set(CMAKE_CXX_FLAGS_MINSIZEREL "/Os /DNDEBUG")

  set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS}")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
  set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL}")

  set(CMAKE_EXE_LINKER_FLAGS "/machine:I386")
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/incremental:yes /debug")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/incremental:no")
  set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/incremental:no /debug")
  set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "/incremental:no")

  set(CMAKE_STATIC_LINKER_FLAGS "/machine:I386")

  set(CMAKE_SHARED_LINKER_FLAGS "/machine:I386")
  set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "/incremental:yes /debug")
  set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "/incremental:no")
  set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "/incremental:no /debug")
  set(CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL "/incremental:no")
endif()

find_program(CLANGFORMAT_BIN NAMES clang-format)
if (EXISTS "${CLANGFORMAT_BIN}")
  execute_process(COMMAND "${CLANGFORMAT_BIN}" --version
    OUTPUT_VARIABLE "CLANGFORMAT_VERSION_OUTPUT"
    RESULT_VARIABLE "CLANGFORMAT_RESULT"
  )
  if (CLANGFORMAT_RESULT EQUAL 0 AND CLANGFORMAT_VERSION_OUTPUT MATCHES "version ([0-9\\.]+)")
    set(CLANGFORMAT_VERSION "${CMAKE_MATCH_1}")
    set(CLANGFORMAT_VERSION_REQUIRED "17.0")
    message(DEBUG "Found clang-format version ${CLANGFORMAT_VERSION} (needs ${CLANGFORMAT_VERSION_REQUIRED}")
    if (CLANGFORMAT_VERSION VERSION_GREATER_EQUAL "${CLANGFORMAT_VERSION_REQUIRED}")
      file(GLOB_RECURSE tony2_sources CONFIGURE_DEPENDS
        "${PROJECT_SOURCE_DIR}/TONY2/*.cpp"
        "${PROJECT_SOURCE_DIR}/TONY2/*.h"
      )
      string(REPLACE ";" "\n" tony2_sources_lines "${tony2_sources}")
      file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/tony2_sources.txt" "${tony2_sources_lines}\n")
      add_custom_target(clang-format ${CLANGFORMAT_BIN} -i "--files=${CMAKE_CURRENT_BINARY_DIR}/tony2_sources.txt")
    endif()
  endif()
endif()

find_library(CLANG_LIBRARY NAMES "clang" DOC "libclang from host (NOT target) system")
find_package(Python3 COMPONENTS Interpreter QUIET)
if (CLANG_LIBRARY AND TARGET Python3::Interpreter)
  add_custom_target(ncc
    Python3::Interpreter "${PROJECT_SOURCE_DIR}/tools/ncc/ncc.py"
      --clang-lib "${CLANG_LIBRARY}"
      --recurse
      --style tools/ncc/ncc.style
      --skip tools/ncc/skip.yml
      --definition DEFINE_GUID PASCAL WINAPI FAR BOOL CALLBACK HWND__=HWND
      --include
        util
        TONY2/include
        TONY2
      --path TONY2
    COMMENT "Verifying naming conventions (ncc)"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
  )
endif()

reccmp_configure()
