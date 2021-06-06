project(zz-path-planning)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

#set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")


find_package(PkgConfig REQUIRED)
find_package(nanomsg CONFIG REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

find_package(Pangolin 0.4 REQUIRED)
include_directories(${Pangolin_INCLUDE_DIRS})

find_package(PCL REQUIRED)
include_directories(${PCL_INCLUDE_DIRS})
link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

add_subdirectory(replaykit)

add_definitions(-DTARGET_PC)

add_executable(
    sky_detector
    ${PATH_PLANNING_ROOT}/src/tools/sky_detector/main.cpp 
    ${PATH_PLANNING_SOURCES}
)

target_include_directories(sky_detector
    PUBLIC
    ${PATH_PLANNING_INCS}
    ${PROTOBUF_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/include/opencv4
)

target_link_libraries(sky_detector
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_videoio.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_video.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_highgui.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_imgcodecs.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_calib3d.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_features2d.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_imgproc.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_core.a

    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibjpeg-turbo.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibtiff.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibpng.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibwebp.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libade.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libIlmImf.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibjasper.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libquirc.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libzlib.a
    
    ${GTK3_LIBRARIES}
    ${Pangolin_LIBRARIES}
)

add_executable(
    zz_path_planning
    ${PATH_PLANNING_SRCS}
    ${PC_PATH_PLANNING_SRCS}
    ${PROTO_SRCS}
)

target_include_directories(zz_path_planning
    PUBLIC
    ${PATH_PLANNING_INCS}
    ${PC_PATH_PLANNING_INCS}
    ${PROTOBUF_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/include/opencv4
    ${catkin_INCLUDE_DIRS}
    ${OCTOMAP_INCLUDE_DIRS}
)

target_link_libraries(zz_path_planning replaykit nanomsg rt dl ${PROTOBUF_LIBRARIES} 
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_ximgproc.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_videoio.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_video.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_highgui.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_imgcodecs.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_calib3d.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_xfeatures2d.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_features2d.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_imgproc.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_core.a
    
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibjpeg-turbo.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibtiff.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibpng.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibwebp.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libade.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libIlmImf.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibjasper.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libquirc.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libzlib.a

    ${GTK3_LIBRARIES}
    ${Pangolin_LIBRARIES}

    nanomsg
)

add_executable(
    binocular_overlap_calculator
    ${PATH_PLANNING_ROOT}/src/tools/binocular_overlap_calculator/main.cpp 
    ${PATH_PLANNING_ROOT}/src/common/CalibrationParser.cpp
    ${PROTO_SRCS}
)

target_include_directories(binocular_overlap_calculator
    PUBLIC
    ${PATH_PLANNING_INCS}
    ${PC_PATH_PLANNING_INCS}
    ${PROTOBUF_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/include/opencv4
)

target_link_libraries(binocular_overlap_calculator replaykit nanomsg rt dl ${PROTOBUF_LIBRARIES} 
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_ximgproc.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_videoio.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_video.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_highgui.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_imgcodecs.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_calib3d.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_xfeatures2d.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_features2d.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_imgproc.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_core.a
    
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibjpeg-turbo.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibtiff.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibpng.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibwebp.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libade.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libIlmImf.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibjasper.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libquirc.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libzlib.a

    ${GTK3_LIBRARIES}
    ${Pangolin_LIBRARIES}

    nanomsg
)

add_executable(
    color_disparity_image
    ${PATH_PLANNING_ROOT}/src/tools/disparity_generator/color_disparity_image.cpp 
)

target_include_directories(color_disparity_image
    PUBLIC
    ${PATH_PLANNING_ROOT}/include
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/include/opencv4
)

target_link_libraries(color_disparity_image rt dl nanomsg
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_ximgproc.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_videoio.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_video.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_highgui.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_imgcodecs.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_calib3d.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_xfeatures2d.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_features2d.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_imgproc.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_core.a
    
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibjpeg-turbo.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibtiff.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibpng.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibwebp.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libade.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libIlmImf.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibjasper.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libquirc.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libzlib.a
    
    ${GTK3_LIBRARIES}
)

add_executable(
    cpu_stereo_matching
    ${PATH_PLANNING_ROOT}/src/stereo_matcher/SGBMStereoMatcherCPU.cpp
    ${PATH_PLANNING_ROOT}/src/tools/disparity_generator/cpu_stereo_matching.cpp 
)

target_include_directories(cpu_stereo_matching
    PUBLIC
    ${PATH_PLANNING_INCS}
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/include/opencv4
)

target_link_libraries(cpu_stereo_matching rt dl nanomsg
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_ximgproc.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_videoio.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_video.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_highgui.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_imgcodecs.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_calib3d.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_xfeatures2d.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_features2d.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_imgproc.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_core.a
    
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibjpeg-turbo.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibtiff.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibpng.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibwebp.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libade.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libIlmImf.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibjasper.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libquirc.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libzlib.a
    
    ${GTK3_LIBRARIES}
)


add_executable(
    images_player
    ${PATH_PLANNING_ROOT}/src/tools/disparity_generator/images_player.cpp 
)

target_include_directories(images_player
    PUBLIC
    ${PATH_PLANNING_ROOT}/include
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/include/opencv4
)

target_link_libraries(images_player rt dl nanomsg
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_ximgproc.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_videoio.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_video.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_highgui.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_imgcodecs.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_calib3d.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_xfeatures2d.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_features2d.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_imgproc.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_core.a
    
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibjpeg-turbo.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibtiff.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibpng.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibwebp.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libade.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libIlmImf.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibjasper.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libquirc.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libzlib.a
    
    ${GTK3_LIBRARIES}
)

add_executable(
    binocular_viewer
    ${PATH_PLANNING_ROOT}/src/tools/binocular_viewer/BinocularViewer.cpp 
)

target_include_directories(binocular_viewer
    PUBLIC
    ${PATH_PLANNING_INCS}
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/include/opencv4
)

target_link_libraries(binocular_viewer rt dl nanomsg
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_ximgproc.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_videoio.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_video.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_highgui.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_imgcodecs.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_calib3d.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_xfeatures2d.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_features2d.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_imgproc.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_core.a
    
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibjpeg-turbo.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibtiff.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibpng.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibwebp.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libade.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libIlmImf.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibjasper.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libquirc.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libzlib.a
    
    ${GTK3_LIBRARIES}
)

add_executable(
    simple_sim
    ${SIM_SOUIRCES}
    ${PATH_PLANNING_SOURCES}
)

target_include_directories(simple_sim
    PUBLIC
    ${PATH_PLANNING_INCS}
    ${PROTOBUF_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/include/opencv4
)

target_link_libraries(simple_sim
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_videoio.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_video.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_highgui.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_imgcodecs.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_calib3d.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_features2d.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_imgproc.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/libopencv_core.a
    
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibjpeg-turbo.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibtiff.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibpng.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibwebp.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libade.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libIlmImf.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/liblibjasper.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libquirc.a
    ${PATH_PLANNING_ROOT}/depends/opencv/x86_64/static/lib/opencv4/3rdparty/libzlib.a

    ${GTK3_LIBRARIES}

    ${Pangolin_LIBRARIES}
    ${PCL_LIBRARIES}
)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/build/)
install(FILES ${PATH_PLANNING_ROOT}/src/tools/simulator/simulator.cfg DESTINATION .)
install(FILES ${PATH_PLANNING_ROOT}/path_planning.prototxt DESTINATION .)
# if(${CMAKE_BUILD_TYPE} STREQUAL Release)
#     install(FILES ${PATH_PLANNING_ROOT}/build/simple_sim DESTINATION release)
#     install(FILES ${PATH_PLANNING_ROOT}/src/simulator/simulator.cfg DESTINATION release)
# elseif(${CMAKE_BUILD_TYPE} STREQUAL Debug)
#     install(FILES ${PATH_PLANNING_ROOT}/build/simple_sim DESTINATION debug)
#     install(FILES ${PATH_PLANNING_ROOT}/src/simulator/simulator.cfg DESTINATION debug)
# endif()