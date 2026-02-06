set(ENGINE_DIR C:/ShellEngine/out/build/x64-release/bin)
set(PROJECT_NAME "New Project")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

set(EDITOR_TGT ShellEngineUserEditor)
set(CLIENT ShellEngineUser)
set(SERVER ShellEngineUserServer)
set(TARGETS ${CLIENT} ${SERVER})

cmake_minimum_required (VERSION 3.12)

project (
 ${PROJECT_NAME}
 VERSION 0.1
 LANGUAGES C CXX
)

include(FetchContent)
include(GenerateExportHeader)

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

add_subdirectory(Source)
add_subdirectory(EditorSource)
add_subdirectory(External/SQLite)