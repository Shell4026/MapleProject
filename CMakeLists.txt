if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(ENGINE_DIR F:\C++\ShellEngine\out\build\x64-Debug\bin)
else()
	set(ENGINE_DIR F:\C++\ShellEngine\out\build\x64-Release\bin)
endif()
set(PROJECT_NAME "New Project")

set(CLIENT ShellEngineUser)
set(SERVER ShellEngineUserServer)
set(TARGETS ${CLIENT} ${SERVER})

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

include(FetchContent)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY "https://github.com/fmtlib/fmt"
  GIT_TAG "10.2.1"
  )
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
  GLM
  GIT_REPOSITORY "https://github.com/g-truc/glm"
  GIT_TAG "1.0.1"
  )
FetchContent_MakeAvailable(GLM)

FETCHCONTENT_DECLARE(
	JSON
	GIT_REPOSITORY "https://github.com/nlohmann/json"
	GIT_TAG "v3.11.3"
)
FETCHCONTENT_MAKEAVAILABLE(JSON)


foreach(TGT ${TARGETS})
	add_library(${TGT} SHARED ${SRC} ${PACKET_SRC} ${AI_SRC} ${HEADERS} ${PACKET_HEADERS} ${AI_HEADERS} ${CMAKE_CURRENT_SOURCE_DIR}/pch.h)

	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		target_compile_definitions(${TGT} PUBLIC SH_DEBUG)
	endif()
	if ("${TGT}" STREQUAL "${SERVER}")
		target_compile_definitions(${SERVER} PUBLIC SH_SERVER)
	endif()

	target_precompile_headers(${TGT} PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/pch.h)
	target_compile_definitions(${TGT} PUBLIC SH_PROJECT_NAME="${PROJECT_NAME}")

	target_include_directories(${TGT} PRIVATE ${ENGINE_DIR}/include)
	target_include_directories(${TGT} PRIVATE ${CMAKE_SOURCE_DIR}/include/external)

	target_link_directories(${TGT} PRIVATE ${ENGINE_DIR})
	target_link_directories(${TGT} PRIVATE ${ENGINE_DIR}/lib)

	target_link_libraries(${TGT} PRIVATE ShellEngineCore)
	target_link_libraries(${TGT} PRIVATE ShellEngineRender)
	target_link_libraries(${TGT} PRIVATE ShellEngineGame)
	target_link_libraries(${TGT} PRIVATE ShellEnginePhysics)
	target_link_libraries(${TGT} PRIVATE ShellEngineWindow)
	target_link_libraries(${TGT} PRIVATE ShellEngineNetwork)
	target_link_libraries(${TGT} PRIVATE IMGUI)
	target_link_libraries(${TGT} PRIVATE fmt)
	target_link_libraries(${TGT} PRIVATE glm)
	target_link_libraries(${TGT} PRIVATE nlohmann_json)
endforeach()