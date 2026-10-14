cmake_minimum_required(VERSION 3.20)
project(samp_client_re CXX C)

# SA-MP 0.3.DL-R1 clean-room client reimplementation.
# Original: samp.dll PE x86 (SHA256 bccdb297464bd382625635be25585df07a8fa6668bc0015650708e3eb4ffcd4b)
# This tree builds with MinGW-w64 x64 out of the box (logic + DLL shell).
# For real GTA:SA injection you MUST build 32-bit (GTA SA is x86):
#   - MSVC: -A Win32, or
#   - MinGW i686: -DSAMP_X86=ON with i686-w64-mingw32-g++ in PATH.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(SAMP_X86 "Build 32-bit x86 (needs multilib/i686 toolchain)" OFF)
option(SAMP_BUILD_LOADER "Build samp_loader.exe (launcher stub)" ON)
option(SAMP_BUILD_TESTS "Build smoke test" ON)

if(SAMP_X86)
  add_compile_options(-m32)
  add_link_options(-m32)
endif()

if(MINGW)
  add_compile_options(-Wall -Wextra -Wno-unused-parameter)
endif()

include_directories(include)

set(SAMP_SOURCES
  src/core/dllmain.cpp
  src/core/archive.cpp
  src/core/exception.cpp
  src/core/hooks_detour.cpp
  src/core/version.cpp
  src/core/version_impl.cpp
  src/net/bitstream.cpp
  src/net/packets.cpp
  src/net/rakpeer.cpp
  src/net/netgame.cpp
  src/net/rpc.cpp
  src/game/game.cpp
  src/game/pools.cpp
  src/game/player.cpp
  src/game/vehicle.cpp
  src/game/gta_patches.cpp
  src/gui/d3dhook.cpp
  src/gui/chat.cpp
  src/gui/gui_rest.cpp
  src/gui/dialog.cpp
  src/gui/scoreboard.cpp
  src/gui/textdraw.cpp
  src/gui/fonts.cpp
  src/gui/screenshot.cpp
  src/audio/audio.cpp
  src/input/input.cpp
  src/util/config.cpp
  src/util/crash.cpp
  src/util/string_utils.cpp
)

add_library(samp_client SHARED ${SAMP_SOURCES})
target_include_directories(samp_client PRIVATE include)
target_link_libraries(samp_client PRIVATE ws2_32 winmm psapi)
set_target_properties(samp_client PROPERTIES OUTPUT_NAME "samp_client" PREFIX "" SUFFIX ".dll")

if(SAMP_BUILD_LOADER)
  add_executable(samp_loader loader/loader.cpp src/util/config.cpp src/util/string_utils.cpp)
  target_include_directories(samp_loader PRIVATE include)
  target_link_libraries(samp_loader PRIVATE psapi)
endif()

if(SAMP_BUILD_TESTS)
  add_executable(samp_smoke tests/smoke.cpp)
  target_include_directories(samp_smoke PRIVATE include)
  target_link_libraries(samp_smoke PRIVATE samp_client)
endif()
