cmake_minimum_required (VERSION 3.9.0)
project (BsZenLib)

option(BSZENLIB_DOWNLOAD_BSF_BINARIES "Download and use precompiled binaries for bsf." OFF)

# Add necessary compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
   CMAKE_CXX_COMPILER_ID MATCHES "AppleClang" OR
   CMAKE_CXX_COMPILER_ID MATCHES "GNU")
   # This is what the example file does, is this really needed at a global scale?
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -g")
endif()

# Make sure to use the C++14 standard
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

###############################################################################
#                       Optional Prebuilt bs:f Download                       #
###############################################################################
if (BSZENLIB_DOWNLOAD_BSF_BINARIES)

  message(STATUS "Downloading prebuilt bs:f...")

  include(ExternalProject)

  set(bsf_INSTALL_DIR ${CMAKE_BINARY_DIR}/bsf-binaries)

  if (WIN32)
    ExternalProject_Add(BsfBinaries
      SOURCE_DIR ${bsf_INSTALL_DIR}
      URL        http://nightly.bsframework.io/releases/bsf_v1.0.0b_win64.zip
      CONFIGURE_COMMAND ""
      BUILD_COMMAND     ""
      INSTALL_COMMAND   ""
      )
  elseif (APPLE)
    ExternalProject_Add(BsfBinaries
      SOURCE_DIR ${bsf_INSTALL_DIR}
      URL        http://nightly.bsframework.io/releases/bsf_v1.0.0b_osx.tar.gz
      CONFIGURE_COMMAND ""
      BUILD_COMMAND     ""
      INSTALL_COMMAND   ""
      )
  elseif (UNIX)
    ExternalProject_Add(BsfBinaries
      SOURCE_DIR ${bsf_INSTALL_DIR}
      URL        http://nightly.bsframework.io/releases/bsf_v1.0.0b_linux.tar.gz
      CONFIGURE_COMMAND ""
      BUILD_COMMAND     ""
      INSTALL_COMMAND   ""
      )
  else()
    message(FATAL_ERROR "No prebuilt binaries available for this platform!")
  endif()

endif()

###############################################################################
#                            Add external libraries                           #
###############################################################################

# Let CMake know where to find the Findbsf.cmake file (at current folder)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

# Find bsf libraries and headers
find_package(bsf REQUIRED)

add_subdirectory(lib/ZenLib)

###############################################################################
#                               Add main library                              #
###############################################################################
add_library(BsZenLib
  src/ImportStaticMesh.cpp
  src/ImportTexture.cpp
  src/ImportZEN.cpp
  )

target_link_libraries(BsZenLib PRIVATE bsf zenload vdfs utils)
target_include_directories(BsZenLib PRIVATE include/BsZenLib)

target_link_libraries(BsZenLib PUBLIC vdfs)
target_include_directories(BsZenLib PUBLIC include)

###############################################################################
#                               Add Samples, etc                              #
###############################################################################
add_subdirectory(samples)
