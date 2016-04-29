cmake_minimum_required(VERSION 2.8)
project(argos3_bullet)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(src/bullet/src)
include_directories(src)
include_directories(.)

find_package(Qt4 COMPONENTS QTCORE QTGUI)
include_directories(${QT_INCLUDES})

find_package(Lua52 REQUIRED)
include_directories(${LUA_INCLUDE_DIR})

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

file(GLOB_RECURSE BULLET_SOURCE_FILES src/bullet/src/*.cpp)

find_package(PkgConfig)
pkg_check_modules(ARGOS REQUIRED argos3_simulator)
set(ARGOS_PREFIX ${ARGOS_PREFIX} CACHE INTERNAL "")
link_directories(${ARGOS_LIBRARY_DIRS})

#find_library(ARGOS_LIB_SIM argos3core_simulator /usr/local/lib/argos3)
#find_library(ARGOS_LIB_VIS argos3plugin_simulator_qtopengl /usr/local/lib/argos3)

set(BULLET_PLUGIN_SOURCES
        src/CBulletModel.cpp
        src/CBulletCubeModel.cpp
        src/CBulletCylinderModel.cpp
        src/CBulletEngine.cpp
        src/CSphereEntity.cpp
        src/CBulletSphereModel.cpp
        src/CQTOpenGLBall.cpp
        src/tinyobjloader/tiny_obj_loader.cpp
        src/CMultibodyEntity.cpp
        src/MeshInfo.cpp
        src/MultibodyEntityDatabase.cpp
        src/CMotorActuatorEntity.cpp
        src/CLinearActuatorEntity.cpp
        src/CBulletMotorModel.cpp
        src/CQTOpenGLMultibodyLinkEntity.cpp
        src/CBulletMultibodyLink.cpp
        src/CMultibodyLinkEntity.cpp
        src/CBulletMultibodyEntity.cpp
        src/CQTOpenGLMultibodyEntity.cpp
        ${BULLET_SOURCE_FILES})

add_library(argos3plugin_bullet SHARED
  ${BULLET_PLUGIN_SOURCES})
target_link_libraries(argos3plugin_bullet argos3core_simulator argos3plugin_simulator_qtopengl ${OPENGL_LIBRARIES})
