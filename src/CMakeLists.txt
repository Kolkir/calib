set(PROJECT_NAME calib)

if(NOT CMAKE_COMPILER_IS_GNUCXX)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/gui.cxx ${CMAKE_CURRENT_SOURCE_DIR}/gui.h
                       COMMAND ../build/fltk/bin/Release/fluid -c ${CMAKE_CURRENT_SOURCE_DIR}/gui.fl
                       DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/gui.fl
                       WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
else()
    add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/gui.cxx ${CMAKE_CURRENT_SOURCE_DIR}/gui.h
                       COMMAND ${PROJECT_BINARY_DIR}/../fltk/bin/fluid -c ${CMAKE_CURRENT_SOURCE_DIR}/gui.fl
                       DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/gui.fl
                       WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

add_custom_target(GENFLTK ALL DEPENDS gui.cxx)

if(WIN32)
    set(FLTK_PLATFORM_DEPENDENT_LIBS wsock32 comctl32)
else()
    set(FLTK_PLATFORM_DEPENDENT_LIBS X11 pthread Xinerama Xft Xext)
endif(WIN32)

set(SOURCE_FILES
    main.cpp
    PatternImageBox.h
    PatternImageBox.cpp
    ImageBox.h
    ImageBox.cpp
    )

source_group("Source Files" FILES ${SOURCE_FILES})

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_SECURE_SCL=0 -D_CRT_SECURE_NO_WARNINGS -DNOMINMAX")
endif()

if(CMAKE_COMPILER_IS_GNUCXX AND WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--enable-auto-import")
endif()

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/external/fltk/)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/include)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/modules/core/include)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/modules/calib3d/include)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/modules/features2d/include)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/modules/flann/include)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/modules/imgproc/include)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/modules/highgui/include)

if (CMAKE_COMPILER_IS_GNUCXX)
   link_directories (${PROJECT_BINARY_DIR}/../fltk/lib)
else()
   link_directories (../fltk/lib/${CMAKE_BUILD_TYPE})
   link_directories (../opencv/lib/${CMAKE_BUILD_TYPE})
   link_directories (../opencv/3rdparty/lib/${CMAKE_BUILD_TYPE})
endif()

add_executable(${PROJECT_NAME} WIN32 ${SOURCE_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/gui.cxx)

if (CMAKE_COMPILER_IS_GNUCXX)
   target_link_libraries(${PROJECT_NAME} debug fltk optimized fltk)
   target_link_libraries(${PROJECT_NAME} debug fltk_images optimized fltk_images)
   target_link_libraries(${PROJECT_NAME} debug fltk_jpeg optimized fltk_jpeg)
   target_link_libraries(${PROJECT_NAME} debug fltk_png optimized fltk_png)
   target_link_libraries(${PROJECT_NAME} debug fltk_z optimized fltk_z)
else()
   target_link_libraries(${PROJECT_NAME} debug fltkd optimized fltk)
   target_link_libraries(${PROJECT_NAME} debug fltkimagesd optimized fltkimages)
   target_link_libraries(${PROJECT_NAME} debug fltkjpegd optimized fltkjpeg)
   target_link_libraries(${PROJECT_NAME} debug fltkpng optimized fltkpng)
   target_link_libraries(${PROJECT_NAME} debug zlibd optimized zlib)
   target_link_libraries(${PROJECT_NAME} debug opencv_calib3d240d optimized opencv_calib3d240)
   target_link_libraries(${PROJECT_NAME} debug opencv_core240d optimized opencv_core240)
   target_link_libraries(${PROJECT_NAME} debug opencv_highgui240d optimized opencv_highgui240)
   target_link_libraries(${PROJECT_NAME} debug opencv_imgproc240d optimized opencv_imgproc240)
   target_link_libraries(${PROJECT_NAME} debug opencv_flann240d optimized opencv_flann240)
   target_link_libraries(${PROJECT_NAME} debug opencv_features2d240d optimized opencv_features2d240)
   target_link_libraries(${PROJECT_NAME} debug libjasperd optimized libjasper)
   target_link_libraries(${PROJECT_NAME} debug libtiffd optimized libtiff)
endif()
target_link_libraries(${PROJECT_NAME} ${FLTK_PLATFORM_DEPENDENT_LIBS})

