add_library(sqlite3 STATIC ${CMAKE_CURRENT_SOURCE_DIR}/sqlite3.c ${CMAKE_CURRENT_SOURCE_DIR}/sqlite3.h)

target_include_directories(sqlite3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})