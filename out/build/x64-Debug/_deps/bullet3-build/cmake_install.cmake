# Install script for directory: C:/Users/Admin/Desktop/UnchartedBlockout/out/build/x64-Debug/_deps/bullet3-src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Users/Admin/Desktop/UnchartedBlockout/out/install/x64-Debug")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/bullet" TYPE FILE FILES
    "C:/Users/Admin/Desktop/UnchartedBlockout/out/build/x64-Debug/_deps/bullet3-src/UseBullet.cmake"
    "C:/Users/Admin/Desktop/UnchartedBlockout/out/build/x64-Debug/_deps/bullet3-build/BulletConfig.cmake"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("C:/Users/Admin/Desktop/UnchartedBlockout/out/build/x64-Debug/_deps/bullet3-build/examples/cmake_install.cmake")
  include("C:/Users/Admin/Desktop/UnchartedBlockout/out/build/x64-Debug/_deps/bullet3-build/Extras/cmake_install.cmake")
  include("C:/Users/Admin/Desktop/UnchartedBlockout/out/build/x64-Debug/_deps/bullet3-build/src/cmake_install.cmake")
  include("C:/Users/Admin/Desktop/UnchartedBlockout/out/build/x64-Debug/_deps/bullet3-build/test/cmake_install.cmake")

endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/Users/Admin/Desktop/UnchartedBlockout/out/build/x64-Debug/_deps/bullet3-build/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
