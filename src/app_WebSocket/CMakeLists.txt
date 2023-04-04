cmake_minimum_required (VERSION 3.15)

project (WebSocket)

option(USE_STANDALONE_ASIO "set ON to use standalone Asio instead of Boost.Asio" OFF)
option(BUILD_TESTING "set ON to build library tests" OFF)

if(NOT MSVC)
    add_compile_options(-std=c++11 -Wall -Wextra -Wsign-conversion)
else()
    add_compile_options(/W1)
endif()

file(GLOB_RECURSE CURRENT_HEADERS  *.h *.hpp *.cuh)
file(GLOB CURRENT_SOURCES  *.c *.cpp *.cu)

source_group("Include" FILES ${CURRENT_HEADERS}) 
source_group("Source" FILES ${CURRENT_SOURCES}) 

# link_directories(${CUDA_LIB_DIR}) 

# create static library
add_library(${PROJECT_NAME} STATIC ${CURRENT_HEADERS} ${CURRENT_SOURCES})

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} INTERFACE ${CMAKE_THREAD_LIBS_INIT})

# TODO 2020 when Debian Jessie LTS ends:
# Remove Boost system, thread, regex components; use Boost::<component> aliases; remove Boost target_include_directories
if(USE_STANDALONE_ASIO)
    target_compile_definitions(${PROJECT_NAME} INTERFACE USE_STANDALONE_ASIO)
    include(CheckIncludeFileCXX)
    CHECK_INCLUDE_FILE_CXX(asio.hpp HAVE_ASIO)
    if(NOT HAVE_ASIO)
        message(FATAL_ERROR "Standalone Asio not found")
    endif()
else()
    find_package(Boost 1.54.0 COMPONENTS system thread coroutine context REQUIRED)
    target_link_libraries(${PROJECT_NAME} INTERFACE ${Boost_LIBRARIES})
    target_include_directories(${PROJECT_NAME} INTERFACE ${Boost_INCLUDE_DIR})
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
        target_compile_definitions(${PROJECT_NAME} INTERFACE USE_BOOST_REGEX)
        find_package(Boost 1.54.0 COMPONENTS regex REQUIRED)
        target_link_libraries(${PROJECT_NAME} INTERFACE ${Boost_LIBRARIES})
        target_include_directories(${PROJECT_NAME} INTERFACE ${Boost_INCLUDE_DIR})
    endif()
endif()
if(WIN32)
    target_link_libraries(${PROJECT_NAME} INTERFACE ws2_32 wsock32)
endif()

if(APPLE)
    set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl")
endif()
find_package(OpenSSL REQUIRED)
target_link_libraries(${PROJECT_NAME} INTERFACE ${OPENSSL_LIBRARIES})
target_include_directories(${PROJECT_NAME} INTERFACE ${OPENSSL_INCLUDE_DIR})

# # If ${PROJECT_NAME} is not a sub-project:
# if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
#     add_executable(ws_examples ws_examples.cpp)
#     target_link_libraries(ws_examples ${PROJECT_NAME})
#     find_package(Boost 1.54.0 COMPONENTS system thread filesystem REQUIRED)
#     target_link_libraries(ws_examples ${Boost_LIBRARIES})
#     target_include_directories(ws_examples PRIVATE ${Boost_INCLUDE_DIR})
#     if(OPENSSL_FOUND)
#         add_executable(wss_examples wss_examples.cpp)
#         target_link_libraries(wss_examples ${PROJECT_NAME})
#         target_link_libraries(wss_examples ${Boost_LIBRARIES})
#         target_include_directories(wss_examples PRIVATE ${Boost_INCLUDE_DIR})
#     endif()
    
#     set(BUILD_TESTING ON)
    
#     install(FILES server_ws.hpp client_ws.hpp server_wss.hpp client_wss.hpp crypto.hpp utility.hpp status_code.hpp DESTINATION include/${PROJECT_NAME})
# endif()

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()