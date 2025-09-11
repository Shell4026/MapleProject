set(ENGINE_DIR F:\C++\ShellEngine\out\build\x64-Release\bin)
set(PROJECT_NAME "New Project")
set(SERVER_SIDE FALSE)

file(GLOB SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp)
file(GLOB PACKET_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Source/Packet/*.cpp)
file(GLOB AI_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Source/AI/*.cpp)

file(GLOB HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h  ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.hpp)
file(GLOB PACKET_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Source/Packet/*.h ${CMAKE_CURRENT_SOURCE_DIR}/Source/Packet/*.hpp)
file(GLOB AI_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Source/AI/*.h)

cmake_minimum_required (VERSION 3.8)

project (
 ${PROJECT_NAME}
 VERSION 0.1
 LANGUAGES CXX
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
if(WIN32)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
elseif(UNIX)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

add_library(ShellEngineUser SHARED ${SRC} ${PACKET_SRC} ${AI_SRC} ${HEADERS} ${PACKET_HEADERS} ${AI_HEADERS})
add_library(ShellEngine::User ALIAS ShellEngineUser)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(ShellEngineUser PUBLIC SH_DEBUG)
endif()
if (SERVER_SIDE)
	target_compile_definitions(ShellEngineUser PUBLIC SH_SERVER)
endif()

target_precompile_headers(ShellEngineUser PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/pch.h)
target_compile_definitions(ShellEngineUser PUBLIC SH_PROJECT_NAME="${PROJECT_NAME}")

target_include_directories(ShellEngineUser PRIVATE ${ENGINE_DIR}/include)
target_include_directories(ShellEngineUser PRIVATE ${CMAKE_SOURCE_DIR}/include/external)

target_link_directories(ShellEngineUser PRIVATE ${ENGINE_DIR})
target_link_directories(ShellEngineUser PRIVATE ${ENGINE_DIR}/lib)

target_link_libraries(ShellEngineUser PRIVATE ShellEngineCore)
target_link_libraries(ShellEngineUser PRIVATE ShellEngineRender)
target_link_libraries(ShellEngineUser PRIVATE ShellEngineGame)
target_link_libraries(ShellEngineUser PRIVATE ShellEnginePhysics)
target_link_libraries(ShellEngineUser PRIVATE ShellEngineWindow)
target_link_libraries(ShellEngineUser PRIVATE ShellEngineNetwork)
target_link_libraries(ShellEngineUser PRIVATE IMGUI)

include(FetchContent)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY "https://github.com/fmtlib/fmt"
  GIT_TAG "10.2.1"
  )
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
FetchContent_MakeAvailable(fmt)
target_link_libraries(ShellEngineUser PRIVATE fmt)

FetchContent_Declare(
  GLM
  GIT_REPOSITORY "https://github.com/g-truc/glm"
  GIT_TAG "1.0.1"
  )
FetchContent_MakeAvailable(GLM)
target_link_libraries(ShellEngineUser PRIVATE glm)

FETCHCONTENT_DECLARE(
	JSON
	GIT_REPOSITORY "https://github.com/nlohmann/json"
	GIT_TAG "v3.11.3"
)
FETCHCONTENT_MAKEAVAILABLE(JSON)
target_link_libraries(ShellEngineUser PRIVATE nlohmann_json)