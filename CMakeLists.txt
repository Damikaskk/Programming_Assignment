cmake_minimum_required(VERSION 3.29)
project(Programing_Assigment C)

set(CMAKE_C_STANDARD 23)

set(SQLite3_INCLUDE_DIR "/usr/local/opt/sqlite3/include")
set(SQLite3_LIBRARY "/usr/local/opt/sqlite3/lib/libsqlite3.dylib")

add_executable(Programing_Assigment main.c)

if(EXISTS ${SQLite3_LIBRARY} AND EXISTS ${SQLite3_INCLUDE_DIR})
    target_include_directories(Programing_Assigment PRIVATE ${SQLite3_INCLUDE_DIR})
    target_link_libraries(Programing_Assigment PRIVATE ${SQLite3_LIBRARY})
else()
    message(FATAL_ERROR "SQLite3 not found in the specified directories!")
endif()
