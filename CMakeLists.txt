cmake_minimum_required(VERSION 3.16)
project(UnchartedBlockout CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# ---- raylib ----
FetchContent_Declare(
  raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(raylib)

# ---- bullet3 ----
FetchContent_Declare(
  bullet3
  GIT_REPOSITORY https://github.com/bulletphysics/bullet3.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(bullet3)

# ---- OpenMesh ----
FetchContent_Declare(
  openmesh
  GIT_REPOSITORY https://www.graphics.rwth-aachen.de:9000/OpenMesh/OpenMesh.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(openmesh)

# ---- مصدر المشروع ----
add_executable(UnchartedBlockout
    src/main.cpp
)

# --- ربط المكتبات ---
target_link_libraries(UnchartedBlockout
    raylib
    BulletDynamics
    BulletCollision
    LinearMath
    OpenMeshCore
    OpenMeshTools
)

# --- ضبط مسارات Bullet ---
# (عشان يلاقي btBulletDynamicsCommon.h في bullet3/src)
target_include_directories(UnchartedBlockout PRIVATE
    ${bullet3_SOURCE_DIR}/src
)
