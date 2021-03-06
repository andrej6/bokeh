cmake_minimum_required (VERSION 3.0)
project(bokeh)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}")
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:\\GraphicsLibraries")

include(ProcessorCount)
ProcessorCount(PROCESSOR_COUNT)

foreach(basename
    bokeh_canvas.cpp
    camera.cpp
    canvas.cpp
    cmj_sampler.cpp
    debug_viz.cpp
    kd_tree.cpp
    lens_assembly.cpp
    main.cpp
    mesh.cpp
    material.cpp
    primitive.cpp
    raytracing.cpp
    scene.cpp
    shader_store.cpp
    threads.cpp
    util.cpp
    )
  set(SRCS ${SRCS} "${CMAKE_CURRENT_SOURCE_DIR}/src/${basename}")
endforeach(basename)

foreach(basename
    dbviz.vert dbviz.frag
    mesh_basic_gouraud.vert mesh_basic_gouraud.frag
    )
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${basename}
    ${CMAKE_CURRENT_BINARY_DIR}/shaders/${basename}
    COPYONLY
    )
endforeach(basename)

add_executable(bokeh ${SRCS})

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLM REQUIRED)
find_package(GLFW REQUIRED)

include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(${GLEW_INCLUDE_DIRS})
include_directories(${GLFW_INCLUDE_DIRS})
include_directories(${GLM_INCLUDE_DIRS})
include_directories(${CMAKE_BINARY_DIR})

target_link_libraries(bokeh
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${GLFW_LIBRARIES}
  )

if (UNIX)
  set_target_properties(bokeh
    PROPERTIES
    COMPILE_FLAGS "-g -Wall -pedantic"
    )
  target_link_libraries(bokeh m pthread)
  add_definitions(-DUNIX)
elseif(MSVC)
  set_target_properties(bokeh
    PROPERTIES
    COMPILE_FLAGS "/Wall /W4"
    )
  add_definitions(-DWINDOWS)
endif()

add_definitions(-DGLM_FORCE_RADIANS -DGLM_FORCE_CTOR_INIT -DGLM_FORCE_INTRINSICS
  -DPROCESSOR_COUNT=${PROCESSOR_COUNT})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
