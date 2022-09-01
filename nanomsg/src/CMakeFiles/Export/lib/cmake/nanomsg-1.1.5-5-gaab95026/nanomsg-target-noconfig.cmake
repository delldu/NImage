#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "nanomsg" for configuration ""
set_property(TARGET nanomsg APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(nanomsg PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libnanomsg.so.5.1.0"
  IMPORTED_SONAME_NOCONFIG "libnanomsg.so.5"
  )

list(APPEND _IMPORT_CHECK_TARGETS nanomsg )
list(APPEND _IMPORT_CHECK_FILES_FOR_nanomsg "${_IMPORT_PREFIX}/lib/libnanomsg.so.5.1.0" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
