#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "OpenMeshCore" for configuration "RelWithDebInfo"
set_property(TARGET OpenMeshCore APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(OpenMeshCore PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/OpenMeshCore_RelWithDebugInfo.lib"
  )

list(APPEND _cmake_import_check_targets OpenMeshCore )
list(APPEND _cmake_import_check_files_for_OpenMeshCore "${_IMPORT_PREFIX}/lib/OpenMeshCore_RelWithDebugInfo.lib" )

# Import target "OpenMeshTools" for configuration "RelWithDebInfo"
set_property(TARGET OpenMeshTools APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(OpenMeshTools PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "C;CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/OpenMeshTools_RelWithDebugInfo.lib"
  )

list(APPEND _cmake_import_check_targets OpenMeshTools )
list(APPEND _cmake_import_check_files_for_OpenMeshTools "${_IMPORT_PREFIX}/lib/OpenMeshTools_RelWithDebugInfo.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
