#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "cppdap::cppdap" for configuration "RelWithDebInfo"
set_property(TARGET cppdap::cppdap APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(cppdap::cppdap PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/cppdap.lib"
  )

list(APPEND _cmake_import_check_targets cppdap::cppdap )
list(APPEND _cmake_import_check_files_for_cppdap::cppdap "${_IMPORT_PREFIX}/lib/cppdap.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
