cmake_minimum_required (VERSION 3.9.0)
project (BsZenLib)

option(BSZENLIB_DOWNLOAD_BSF_BINARIES "Download and use precompiled binaries for bsf." OFF)
set(BSZENLIB_BSF_SUBMODULE_PATH "" CACHE PATH "Path to bsf sources to add as cmake subdirectory")

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

  include(FetchContent)

  set(bsf_INSTALL_DIR ${CMAKE_BINARY_DIR}/bsf-binaries CACHE PATH "" FORCE)

  if (WIN32)
    FetchContent_Declare(bsfBinaries
      SOURCE_DIR ${bsf_INSTALL_DIR}
      URL        https://dilborceisv8p.cloudfront.net/bsf_2019.01.23_win64.zip
      )
  elseif (APPLE)
    FetchContent_Declare(bsfBinaries
      SOURCE_DIR ${bsf_INSTALL_DIR}
      URL        https://dilborceisv8p.cloudfront.net/bsf_2019.01.23_osx.tar.gz
      )
  elseif (UNIX)
    FetchContent_Declare(bsfBinaries
      SOURCE_DIR ${bsf_INSTALL_DIR}
      URL        https://dilborceisv8p.cloudfront.net/bsf_2019.01.23_linux.tar.gz
      )
  else()
    message(FATAL_ERROR "No prebuilt binaries available for this platform!")
  endif()

  FetchContent_Populate(bsfBinaries)
endif()


###############################################################################
#                            Add external libraries                           #
###############################################################################

# Let CMake know where to find the Findbsf.cmake file (at current folder)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

if(BSZENLIB_BSF_SUBMODULE_PATH)
  # Optionally include bsf as cmake submodule for easier development on bsf
  add_subdirectory(${BSZENLIB_BSF_SUBMODULE_PATH} bsf)
else()
  # Find bsf libraries and headers
  find_package(bsf REQUIRED)
endif()

add_subdirectory(lib/ZenLib)

###############################################################################
#                               Add main library                              #
###############################################################################
add_library(BsZenLib
  src/ImportSkeletalMesh.cpp
  src/ImportStaticMesh.cpp
  src/ImportTexture.cpp
  src/ImportMaterial.cpp
  src/ImportZEN.cpp
  src/ImportPath.cpp
  src/ImportAnimation.cpp
  src/ZenResources.cpp
  src/CacheUtility.cpp
  )

target_link_libraries(BsZenLib PRIVATE bsf zenload vdfs utils)
target_include_directories(BsZenLib PRIVATE include/BsZenLib)

target_link_libraries(BsZenLib PUBLIC vdfs)
target_include_directories(BsZenLib PUBLIC include)

###############################################################################
#                               Add Samples, etc                              #
###############################################################################
add_subdirectory(samples)
add_subdirectory(docs-source)
