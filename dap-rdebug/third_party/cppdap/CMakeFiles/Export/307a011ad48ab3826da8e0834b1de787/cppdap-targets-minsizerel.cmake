#----------------------------------------------------------------
# Generated CMake target import file for configuration "MinSizeRel".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "cppdap::cppdap" for configuration "MinSizeRel"
set_property(TARGET cppdap::cppdap APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(cppdap::cppdap PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "CXX"
  IMPORTED_LOCATION_MINSIZEREL "${_IMPORT_PREFIX}/lib/cppdap.lib"
  )

list(APPEND _cmake_import_check_targets cppdap::cppdap )
list(APPEND _cmake_import_check_files_for_cppdap::cppdap "${_IMPORT_PREFIX}/lib/cppdap.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
