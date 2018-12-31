

add_executable(zen-bs
  zen-bs.cpp
  BsFPSCamera.cpp
  BsFPSWalker.cpp
  )

add_executable(zen-bs-vob
  zen-bs-vob.cpp
  BsObjectRotator.cpp
  )

# Link bsf library with your application
target_link_libraries(zen-bs PRIVATE bsf)
target_link_libraries(zen-bs PRIVATE BsZenLib)


# Link bsf library with your application
target_link_libraries(zen-bs-vob PRIVATE bsf)
target_link_libraries(zen-bs-vob PRIVATE BsZenLib)
