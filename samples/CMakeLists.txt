
add_library(samples-common
  common/BsCameraZoomer.cpp
  common/BsFPSCamera.cpp
  common/BsFPSWalker.cpp
  common/BsObjectRotator.cpp
 )

target_include_directories(samples-common PUBLIC common)
target_link_libraries(samples-common PUBLIC bsf BsZenLib)

add_executable(zen-bs zen-bs.cpp)
target_link_libraries(zen-bs samples-common)

add_executable(mesh-viewer mesh-viewer.cpp)
target_link_libraries(mesh-viewer samples-common)

add_executable(npc-viewer npc-viewer.cpp)
target_link_libraries(npc-viewer samples-common)
