cmake_minimum_required(VERSION 3.10)
project(idirt VERSION 1.82)

set(SOURCES src/acct.c src/actions.c src/bootstrap.c src/bprintf.c src/change.c src/climate.c
            src/clone.c src/comm.c src/commands.c src/exit.c src/fight.c src/flags.c src/frob.c
            src/god.c src/log.c src/magic.c src/mailer.c src/main.c src/mobile.c src/move.c
            src/mud.c src/objsys.c src/parse.c src/puff.c src/rooms.c src/sendsys.c src/spell.c
            src/s_socket.c src/timing.c src/uaf.c src/update.c src/utils.c src/ver.c src/viewcom.c
            src/wizard.c src/wizlist.c src/writer.c src/zones.c)

add_executable(generate src/generate.c src/mkdata.c src/utils.c)

add_executable(aberd
    ${SOURCES}
    ${CMAKE_BINARY_DIR}/locations.h
    ${CMAKE_BINARY_DIR}/mobiles.h
    ${CMAKE_BINARY_DIR}/objects.h
    ${CMAKE_BINARY_DIR}/verbs.h
)

add_custom_command(
    OUTPUT
        ${CMAKE_BINARY_DIR}/locations.h
        ${CMAKE_BINARY_DIR}/mobiles.h
        ${CMAKE_BINARY_DIR}/objects.h
    COMMAND $<TARGET_FILE:generate> data ${CMAKE_SOURCE_DIR}/data/generate.conf
    DEPENDS generate
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating world files"
)

add_custom_command(
    OUTPUT
        ${CMAKE_BINARY_DIR}/verbs.h
    COMMAND $<TARGET_FILE:generate>
        verbs
        ${CMAKE_SOURCE_DIR}/data/verbs.src
        ${CMAKE_BINARY_DIR}/verbs
        ${CMAKE_BINARY_DIR}/verbs.h
    DEPENDS generate
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating verb files"
)

target_compile_definitions(aberd PRIVATE
    _ARCH_="${CMAKE_SYSTEM_PROCESSOR}"
    _OS_="${CMAKE_SYSTEM_NAME}"
)
target_include_directories(aberd PRIVATE include ${CMAKE_BINARY_DIR})
target_include_directories(generate PRIVATE include)

target_link_libraries(aberd PRIVATE crypt)
target_link_libraries(generate PRIVATE crypt)

target_include_directories(aberd PRIVATE
    include
    ${CMAKE_BINARY_DIR}
)