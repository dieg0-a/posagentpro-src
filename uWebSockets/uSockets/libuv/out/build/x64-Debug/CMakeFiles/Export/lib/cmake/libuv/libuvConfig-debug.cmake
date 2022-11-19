#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "uv_a" for configuration "Debug"
set_property(TARGET uv_a APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(uv_a PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/uv_a.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS uv_a )
list(APPEND _IMPORT_CHECK_FILES_FOR_uv_a "${_IMPORT_PREFIX}/lib/uv_a.lib" )

# Import target "uv" for configuration "Debug"
set_property(TARGET uv APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(uv PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/lib/uv.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/uv.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS uv )
list(APPEND _IMPORT_CHECK_FILES_FOR_uv "${_IMPORT_PREFIX}/lib/uv.lib" "${_IMPORT_PREFIX}/bin/uv.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
