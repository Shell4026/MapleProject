set(COMMON_DLL_NAME "ShellEngineUser")

file(GLOB_RECURSE CLIENT_SRC CONFIGURE_DEPENDS 
	${CMAKE_CURRENT_SOURCE_DIR}/client/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/common/*.cpp
)
file(GLOB_RECURSE SERVER_SRC CONFIGURE_DEPENDS 
	${CMAKE_CURRENT_SOURCE_DIR}/server/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/common/*.cpp
)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.h  ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

foreach(TGT ${TARGETS})
	if ("${TGT}" STREQUAL "${SERVER}")
		add_library(${TGT} SHARED ${SERVER_SRC} ${HEADERS} ${CMAKE_SOURCE_DIR}/PCH.h)
		target_compile_definitions(${SERVER} PUBLIC SH_SERVER)
		set_target_properties(${TGT} PROPERTIES 
			ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/Server"
			RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/Server"
			LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/Server")
	else()
		add_library(${TGT} SHARED ${CLIENT_SRC} ${HEADERS} ${CMAKE_SOURCE_DIR}/PCH.h)
		set_target_properties(${TGT} PROPERTIES 
			ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/Client"
			RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/Client"
			LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/Client")
	endif()

	set_target_properties(${TGT} PROPERTIES OUTPUT_NAME ${COMMON_DLL_NAME})

	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		target_compile_definitions(${TGT} PUBLIC SH_DEBUG)
	endif()
	target_precompile_headers(${TGT} PRIVATE  ${CMAKE_SOURCE_DIR}/PCH.h)
	target_compile_definitions(${TGT} PUBLIC SH_PROJECT_NAME="${PROJECT_NAME}")

	target_include_directories(${TGT} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
	target_include_directories(${TGT} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
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
