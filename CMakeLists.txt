cmake_minimum_required( VERSION 3.26 )
project( sfml-embedded )
set( CMAKE_CXX_STANDARD 17 )

#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

add_library( ${PROJECT_NAME}
  STATIC
  EmbeddedWindowImplWin32.cpp
  EmbeddedWindowImpl.cpp
  EmbeddedWindow.cpp
  EmbeddedLogger.cpp
)

set( SFML_STATIC_LIBRARIES TRUE )

find_package( SFML 2.6 COMPONENTS graphics window main REQUIRED )

set( INCL_DIRS ${CMAKE_SOURCE_DIR}/embedded )
set( COMPILE_DEFS
  -DNOMINMAX
  -DWINDOWS_LEAN_AND_MEAN
  -DWIN32
  -D_WINDOWS
  -D_UNICODE
  -DUNICODE
)

if( DEFINED SPDLOG_DIR )
  list( APPEND INCL_DIRS ${SPDLOG_DIR} )
  list( APPEND COMPILE_DEFS -DUSING_LOGGER )
endif()

target_include_directories( ${PROJECT_NAME}
  PRIVATE
  ${INCL_DIRS}
)

target_link_libraries( ${PROJECT_NAME}
  PRIVATE
  Rpcrt4.lib
  sfml-graphics
  sfml-window
  sfml-main
)

add_definitions( ${COMPILE_DEFS} )