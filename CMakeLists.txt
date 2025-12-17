cmake_minimum_required(VERSION 3.10)

project(ChessProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Настройка SFML ---
set(SFML_PATH "${CMAKE_SOURCE_DIR}/include/SFML-3.0.2")

if(NOT EXISTS "${SFML_PATH}")
    message(FATAL_ERROR "SFML не найдена по пути: ${SFML_PATH}. Проверьте структуру папок.")
endif()

include_directories("${SFML_PATH}/include")
link_directories("${SFML_PATH}/lib")

# --- Определение исходных файлов ---

# 1. Общие файлы (Логика, которая нужна и Клиенту, и Серверу)
# ВАЖНО: Добавлен position.cpp, так как Move зависит от Position!
set(SHARED_SOURCES
    "Chess/src/core/move.cpp"
    "Chess/src/core/move.h"
    "Chess/src/core/position.cpp" 
    "Chess/src/core/position.h"
    "Chess/src/network/PacketType.h"
)

# 2. Файлы Клиента (Все .cpp, кроме серверных)
file(GLOB_RECURSE ALL_CLIENT_SOURCES 
    "Chess/*.cpp"
    "Chess/*.h"
    "Chess/*.hpp"
)

# Исключаем файлы, которые относятся ТОЛЬКО к запуску сервера
list(REMOVE_ITEM ALL_CLIENT_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/Chess/src/network/ServerMain.cpp"
)

# --- Сборка КЛИЕНТА ---
add_executable(Chess ${ALL_CLIENT_SOURCES})

target_include_directories(Chess PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/Chess"
    "${CMAKE_CURRENT_SOURCE_DIR}/Chess/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/Chess/src/core"
    "${CMAKE_CURRENT_SOURCE_DIR}/Chess/src/network"
    "${CMAKE_CURRENT_SOURCE_DIR}/Chess/src/graphic"
    "${CMAKE_CURRENT_SOURCE_DIR}/Chess/src/menu"
)

target_link_libraries(Chess PRIVATE 
    debug sfml-graphics-d      optimized sfml-graphics
    debug sfml-window-d        optimized sfml-window
    debug sfml-system-d        optimized sfml-system
    debug sfml-audio-d         optimized sfml-audio
    debug sfml-network-d       optimized sfml-network 
    debug sfml-main-d          optimized sfml-main
)

# --- Сборка СЕРВЕРА ---
add_executable(Server
    "Chess/src/network/ServerMain.cpp"
    "Chess/src/network/chessServer.cpp" 
    "Chess/src/network/chessServer.h"
    # Добавляем общие файлы (move.cpp, position.cpp), чтобы линковщик нашел реализацию методов
    ${SHARED_SOURCES} 
)

target_include_directories(Server PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/Chess/src/network"
    "${CMAKE_CURRENT_SOURCE_DIR}/Chess/src/core"
    "${CMAKE_CURRENT_SOURCE_DIR}/Chess" 
)

target_link_libraries(Server PRIVATE
    debug sfml-network-d       optimized sfml-network
    debug sfml-system-d        optimized sfml-system
)

# --- Пост-сборочные команды (Копирование DLL и ассетов) ---
if(WIN32)
    # Копирование DLL для Клиента
    add_custom_command(TARGET Chess POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${SFML_PATH}/bin"
        "$<TARGET_FILE_DIR:Chess>"
        COMMENT "Copying DLLs to Client..."
    )

    # Копирование папки assets для Клиента
    add_custom_command(TARGET Chess POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Chess/assets"
        "$<TARGET_FILE_DIR:Chess>/assets"
        COMMENT "Copying assets to Client..."
    )

    # Копирование DLL для Сервера
    add_custom_command(TARGET Server POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${SFML_PATH}/bin"
        "$<TARGET_FILE_DIR:Server>"
        COMMENT "Copying DLLs to Server..."
    )
endif()