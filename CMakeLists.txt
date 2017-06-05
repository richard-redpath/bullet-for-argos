cmake_minimum_required(VERSION 2.8)
project(argos3_bullet)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(PROJ_SRC_OFFSET plugins/simulator/physics_engines/bullet-for-argos/)
set(BASE_PROJ_DIR src/${PROJ_SRC_OFFSET})

include_directories("${BASE_PROJ_DIR}bullet/src")
include_directories("${BASE_PROJ_DIR}")
include_directories("./src")

find_package(Qt5 COMPONENTS Widgets Gui)
add_definitions(${Qt5Widgets_DEFINITIONS} ${Qt5Gui_DEFINITIONS})
include_directories(${Qt5Widgets_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS})

find_package(Lua52 REQUIRED)
include_directories(${LUA_INCLUDE_DIR})

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

file(GLOB_RECURSE BULLET_SOURCE_FILES "${BASE_PROJ_DIR}bullet/src/*.cpp")
file(GLOB_RECURSE TINYOBJLOADER_SOURCE_FILES "${BASE_PROJ_DIR}tinyobjloader/*.cpp")
file(GLOB PLUGIN_SOURCE_FILES "${BASE_PROJ_DIR}*.cpp")
file(GLOB PLUGIN_HEADER_FILES "${BASE_PROJ_DIR}*.h")

find_package(PkgConfig)
pkg_check_modules(ARGOS REQUIRED argos3_simulator)
set(ARGOS_PREFIX ${ARGOS_PREFIX} CACHE INTERNAL "")
link_directories(${ARGOS_LIBRARY_DIRS})

set(BULLET_PLUGIN_SOURCES
        ${PLUGIN_SOURCE_FILES}
        ${BULLET_SOURCE_FILES}
	${TINYOBJLOADER_SOURCE_FILES})

add_library(argos3plugin_bullet SHARED
  ${BULLET_PLUGIN_SOURCES})

target_link_libraries(argos3plugin_bullet argos3core_simulator argos3plugin_simulator_qtopengl ${OPENGL_LIBRARIES})

install(FILES ${PLUGIN_HEADER_FILES} DESTINATION "${ARGOS_INCLUDEDIR}/argos3/${PROJ_SRC_OFFSET}")
install(TARGETS argos3plugin_bullet LIBRARY DESTINATION ${ARGOS_LIBDIR})
