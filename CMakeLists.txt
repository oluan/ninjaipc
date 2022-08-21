project(ninjaipc)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dist)
set (CMAKE_C_FLAGS "-ansi -pedantic -g")

set(
  SOURCES ${SOURCES}
  src/ninjaview.c
  src/ninjall.c
  src/common/handle.c
  src/common/listen.c
  src/common/request.c
)


if(WIN32)
  set(
    SOURCES ${SOURCES}
    src/win32/sync.c
    src/win32/view.c
  )
else()
  set(
    SOURCES ${SOURCES}
    src/linux/sync.c
    src/linux/view.c
  )
endif()

set(
  SERVER_SOURCES ${SOURCES}
  examples/server.c
)

set(
  CLIENT_SOURCES ${SOURCES}
  examples/client.c
)

add_executable(server ${SERVER_SOURCES})
target_link_libraries(server pthread rt)

add_executable(client ${CLIENT_SOURCES})
target_link_libraries(client pthread rt)
