project(ninjaipc)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dist)

set(
  SOURCES ${SOURCES}
  examples/ipcexample.c
  src/ninjall.c
)

# if(WIN32)
#   set(
#     SOURCES ${SOURCES}
#     src/win32/sync.c
#     src/win32/view.c
#   )
# else()
#   set(
#     SOURCES ${SOURCES}
#     src/linux/sync.c
#     src/linux/view.c
#   )
# endif()

add_executable(test ${SOURCES})
