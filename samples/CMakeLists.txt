

add_executable(zen-bs
  zen-bs.cpp
  BsFPSCamera.cpp
  BsFPSWalker.cpp
  )

target_link_libraries(zen-bs PRIVATE bsf)
target_link_libraries(zen-bs PRIVATE BsZenLib)


add_executable(mesh-viewer
  mesh-viewer.cpp
  BsObjectRotator.cpp
  BsCameraZoomer.cpp
  )

target_link_libraries(mesh-viewer PRIVATE bsf)
target_link_libraries(mesh-viewer PRIVATE BsZenLib)

add_executable(npc-viewer
  npc-viewer.cpp
  BsObjectRotator.cpp
  BsCameraZoomer.cpp
  )

target_link_libraries(npc-viewer PRIVATE bsf)
target_link_libraries(npc-viewer PRIVATE BsZenLib)
