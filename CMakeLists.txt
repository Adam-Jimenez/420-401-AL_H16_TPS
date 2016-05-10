cmake_minimum_required(VERSION 2.8)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTORCC OFF)

find_package(Qt5Widgets QUIET REQUIRED)
find_package(Qt5Gui QUIET REQUIRED)
find_package(Qt5Core QUIET REQUIRED)
find_package(Qt5Network QUIET REQUIRED)
find_package(Qt5Svg QUIET REQUIRED)
IF (Qt5_POSITION_INDEPENDENT_CODE)
    SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
ENDIF (Qt5_POSITION_INDEPENDENT_CODE)
if (PLATFORM_NAME STREQUAL "NaCl" OR PLATFORM_NAME STREQUAL "PNaCl")
    include(qt.nacl.cmake)
endif (PLATFORM_NAME STREQUAL "NaCl" OR PLATFORM_NAME STREQUAL "PNaCl")

include_directories(${Qt5Svg_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS} ${Qt5Core_INCLUDE_DIRS})
add_definitions(${Qt5Svg_DEFINITIONS} ${Qt5Network_DEFINITIONS} ${Qt5Widgets_DEFINITIONS} ${Qt5Gui_DEFINITIONS} ${Qt5Core_DEFINITIONS})
list(INSERT LIBRARIES 0 ${Qt5Svg_LIBRARIES} ${Qt5Network_LIBRARIES} ${Qt5Widgets_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5Core_LIBRARIES})

set(ALIENS_SRCS alien/alien.cpp
                alien/epoe.cpp
                alien/og.cpp
                alien/uqomua.cpp
                alien/yuhq.cpp
                alien/smart.cpp
                alien/proxy.cpp)
set (ALIENS_HDRS alien/alien.h
                 alien/epoe.h
                 alien/og.h
                 alien/uquoma.h
                 alien/yuhq.h
                 alien/smart.h
                 alien/proxy.h)

set(SERVER_MOC_HDRS src/gui.h
                    src/updater.h)
set(SERVER_SRCS server/server.cpp
                server/dummy.cpp
                server/loop.cpp
                src/simulation.cpp
                src/updater.cpp
                src/board.cpp
                src/gui.cpp
                ${ALIENS_SRCS})
set(SERVER_HDRS server/servers.h
                server/server.h
                server/dummy.h
                server/loop.h
                src/simulation.h
                src/protocol.h
                src/board.h
                src/common.h
                src/ptr.h
                src/signal.h
                ${ALIENS_SRCS})

qt5_generate_moc(src/gui.cpp gui.moc)
list (APPEND SERVER_SRCS gui.moc)

if (NOT PLATFORM_NAME STREQUAL "NaCl" AND NOT PLATFORM_NAME STREQUAL "PNaCl")
    qt5_generate_moc(server/tcp.cpp server_tcp.moc)
    list(APPEND SERVER_HDRS server/tcp.h)
    list(APPEND SERVER_SRCS server/main.cpp server/tcp.cpp server_tcp.moc)
else (NOT PLATFORM_NAME STREQUAL "NaCl" AND NOT PLATFORM_NAME STREQUAL "PNaCl")
    list(APPEND SERVER_HDRS server/pp.h server/pp_inst.h)
    list(APPEND SERVER_SRCS server/pp.cpp server/pp_inst.cpp)
endif (NOT PLATFORM_NAME STREQUAL "NaCl" AND NOT PLATFORM_NAME STREQUAL "PNaCl")

QT5_WRAP_CPP(SERVER_MOCS ${SERVER_MOC_HDRS})
QT5_ADD_RESOURCES(SERVER_RSCS rc.qrc)
list(APPEND SERVER_SRCS ${SERVER_MOCS} ${SERVER_RSCS})
add_executable(simul_server${PLATFORM_EXE_SUFFIX} ${SERVER_SRCS} ${SERVER_HDRS})
target_link_libraries(simul_server${PLATFORM_EXE_SUFFIX} ${LIBRARIES})

set(CLIENT_HDRS src/protocol.h
                src/simulation.h
                client/clients.h
                client/client.h
                client/loop.h
                ${ALIENS_SRCS})
set(CLIENT_SRCS client/client.cpp
                client/loop.cpp
                ${ALIENS_SRCS})
if (NOT PLATFORM_NAME STREQUAL "NaCl" AND NOT PLATFORM_NAME STREQUAL "PNaCl")
    qt5_generate_moc(client/tcp.cpp client_tcp.moc)
    list(APPEND CLIENT_HDRS client/tcp.h)
    list(APPEND CLIENT_SRCS client/main.cpp client/tcp.cpp client_tcp.moc)
else (NOT PLATFORM_NAME STREQUAL "NaCl" AND NOT PLATFORM_NAME STREQUAL "PNaCl")
    list(APPEND CLIENT_HDRS client/pp.h client/pp_inst.h)
    list(APPEND CLIENT_SRCS client/pp.cpp client/pp_inst.cpp)
endif (NOT PLATFORM_NAME STREQUAL "NaCl" AND NOT PLATFORM_NAME STREQUAL "PNaCl")

QT5_WRAP_CPP(CLIENT_MOCS ${CLIENTS_HDRS})
list(APPEND CLIENT_SRCS ${CLIENT_MOCS})
add_executable(simul_client${PLATFORM_EXE_SUFFIX} ${CLIENT_SRCS} ${CLIENT_HDRS})
target_link_libraries(simul_client${PLATFORM_EXE_SUFFIX} ${LIBRARIES})
