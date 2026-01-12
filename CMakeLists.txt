if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(ENGINE_DIR C:\ShellEngine\out\build\x64-Debug\bin)
else()
	set(ENGINE_DIR C:\ShellEngine\out\build\x64-Release\bin)
endif()
set(PROJECT_NAME "New Project")

set(CLIENT ShellEngineUser)
set(SERVER ShellEngineUserServer)
set(TARGETS ${CLIENT} ${SERVER})

file(GLOB_RECURSE CLIENT_SRC CONFIGURE_DEPENDS 
	${CMAKE_CURRENT_SOURCE_DIR}/Source/client/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Source/common/*.cpp
)
file(GLOB_RECURSE SERVER_SRC CONFIGURE_DEPENDS 
	${CMAKE_CURRENT_SOURCE_DIR}/Source/server/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Source/common/*.cpp
)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h  ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.hpp)

cmake_minimum_required (VERSION 3.8)

project (
 ${PROJECT_NAME}
 VERSION 0.1
 LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(COMMON_DLL_NAME "ShellEngineUser")

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

add_subdirectory(External/SQLite)

foreach(TGT ${TARGETS})
	if ("${TGT}" STREQUAL "${SERVER}")
		add_library(${TGT} SHARED ${SERVER_SRC} ${HEADERS} ${CMAKE_CURRENT_SOURCE_DIR}/pch.h)
		target_compile_definitions(${SERVER} PUBLIC SH_SERVER)
		set_target_properties(${TGT} PROPERTIES 
			ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/Server"
			RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/Server"
			LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/Server")
	else()
		add_library(${TGT} SHARED ${CLIENT_SRC} ${HEADERS} ${CMAKE_CURRENT_SOURCE_DIR}/pch.h)
		set_target_properties(${TGT} PROPERTIES 
			ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/Client"
			RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/Client"
			LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/Client")
	endif()

	set_target_properties(${TGT} PROPERTIES OUTPUT_NAME ${COMMON_DLL_NAME})

	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		target_compile_definitions(${TGT} PUBLIC SH_DEBUG)
	endif()

	target_precompile_headers(${TGT} PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/pch.h)
	target_compile_definitions(${TGT} PUBLIC SH_PROJECT_NAME="${PROJECT_NAME}")

	target_include_directories(${TGT} PRIVATE ${CMAKE_SOURCE_DIR}/Source/include)
	target_include_directories(${TGT} PRIVATE ${ENGINE_DIR}/include)
	target_include_directories(${TGT} PRIVATE ${CMAKE_SOURCE_DIR}/External)

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
	target_link_libraries(${TGT} PRIVATE sqlite3)
endforeach()