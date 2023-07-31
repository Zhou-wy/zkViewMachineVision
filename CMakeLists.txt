cmake_minimum_required(VERSION 3.15)

project(TrtServer LANGUAGES CUDA CXX)

set(CMAKE_CXX_STANDARD 11)

#cmake在编译过程中打印编印信息
set(CMAKE_VERBOSE_MAKEFILEON ON) 

find_package(OpenCV)
enable_language(CUDA)
set(CMAKE_BUILD_TYPE Debug)


set(CMAKE_CUDA_COMPILER /usr/local/cuda/bin/nvcc)
set(CUDA_HOME /usr/local/cuda)
set(TRT_HOME /home/zwy/TensorRT-7.2.3.4)

# 设置动态链接库路径
set(CUDA_LIB_DIR ${CUDA_HOME}/lib64)
set(TRT_LIB_DIR ${TRT_HOME}/lib)
set(LD_CUDA_LIBS cuda cudart)
set(LD_TRT_LIBS myelin nvcaffe_parser nvinfer nvinfer_plugin nvonnxparser nvparsers)

# 设置输出bin文件路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

set(CMAKE_CXX_FLAGS "-Wno-error=deprecated-declarations -Wno-deprecated-declarations")
# cuda 和 cudnn 头文件
include_directories(${CUDA_HOME}/include)
include_directories(${CUDA_HOME}/targets/x86_64-linux/include)

# TensorRT 头文件
include_directories(${TRT_HOME}/include)
include_directories(${TRT_HOME}/sample)

# OpenCV 头文件
include_directories(${OpenCV_INCLUDE_DIRS})

# FFMpeg 头文件
set(FFMPEG_PATH " /usr/local/ffmpeg")
INCLUDE_DIRECTORIES(${FFMPEG_PATH}/include)
LINK_DIRECTORIES(${FFMPEG_PATH}/lib)


# src 链接库
add_subdirectory(${CMAKE_SOURCE_DIR}/src/TrtLib/common)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/TrtLib/builder)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/TrtLib/infer)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/app_WebSocket)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/app_yolo)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/app_http)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/VideoPusher)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/yolov8)

set(FFMEPG_LIB ${FFMEPG_LIB} avcodec  avdevice  avfilter  avformat  avutil  postproc  swresample  swscale)
set(EXTRA_LIBS ${EXTRA_LIBS}  yolov8 WebSocket http yolo TrtInfer TrtBuilder common RtspToFlv)

link_directories(${CUDA_LIB_DIR} ${TRT_LIB_DIR}) 

# add_executable(ZkViewYolo ${CMAKE_SOURCE_DIR}/main.cpp)
# # 链接动态链接库
# target_link_libraries(ZkViewYolo ${LD_TRT_LIBS} ${LD_CUDA_LIBS} ${OpenCV_LIBS} ${EXTRA_LIBS} ${FFMEPG_LIB})

# add_executable(test_yolo_seg_http ${CMAKE_SOURCE_DIR}/test/test_yolov8_seg.cpp)
# # 链接动态链接库
# target_link_libraries(test_yolo_seg_http ${LD_TRT_LIBS} ${LD_CUDA_LIBS} ${OpenCV_LIBS} ${EXTRA_LIBS})

add_executable(test_yolo_http ${CMAKE_SOURCE_DIR}/test/test_yolo_http.cpp)
# 链接动态链接库
target_link_libraries(test_yolo_http ${LD_TRT_LIBS} ${LD_CUDA_LIBS} ${OpenCV_LIBS} ${EXTRA_LIBS})

add_executable(test_pushvideo ${CMAKE_SOURCE_DIR}/test/test_pushvideo.cpp)
# 链接动态链接库
target_link_libraries(test_pushvideo ${LD_TRT_LIBS} ${LD_CUDA_LIBS} ${OpenCV_LIBS} ${EXTRA_LIBS})
    
if(BUILD_TESTING)
    enable_testing()
    add_executable(test_yolo_http ${CMAKE_SOURCE_DIR}/test/test_yolo_http.cpp)
    # 链接动态链接库
    target_link_libraries(test_yolo_http ${LD_TRT_LIBS} ${LD_CUDA_LIBS} ${OpenCV_LIBS} ${EXTRA_LIBS})

    add_executable(test_websocket ${CMAKE_SOURCE_DIR}/test/test_websocket.cpp)
    # 链接动态链接库
    target_link_libraries(test_websocket ${LD_TRT_LIBS} ${LD_CUDA_LIBS} ${OpenCV_LIBS} ${EXTRA_LIBS})

    add_executable(test_websocket ${CMAKE_SOURCE_DIR}/test/test_websocket.cpp)
    # 链接动态链接库
    target_link_libraries(test_websocket ${LD_TRT_LIBS} ${LD_CUDA_LIBS} ${OpenCV_LIBS} ${EXTRA_LIBS})

    add_executable(test_rtsp2rtmp ${CMAKE_SOURCE_DIR}/test/test_ffmpeg_rtsp2rtmp.cpp)
    # 链接动态链接库
    target_link_libraries(test_rtsp2rtmp ${LD_TRT_LIBS} ${LD_CUDA_LIBS} ${OpenCV_LIBS} ${EXTRA_LIBS})
    
    add_executable(test_json ${CMAKE_SOURCE_DIR}/test/test_json.cpp)
    # 链接动态链接库
    target_link_libraries(test_json common)

    add_executable(test_zk_Instrument ${CMAKE_SOURCE_DIR}/test/test_yolo_Instrument.cpp)
    # 链接动态链接库
    target_link_libraries(test_zk_Instrument ${LD_TRT_LIBS} ${LD_CUDA_LIBS} ${OpenCV_LIBS} ${EXTRA_LIBS} ${FFMEPG_LIB})
endif()
