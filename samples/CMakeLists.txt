

add_executable(zen-bs
  zen-bs.cpp
  BsFPSCamera.cpp
  BsFPSWalker.cpp
  )

# Link bsf library with your application
target_link_libraries(zen-bs PRIVATE bsf)
target_link_libraries(zen-bs PRIVATE BsZenLib)
