cmake_minimum_required(VERSION 3.0)

project(demo)

# static compile
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS} -static-libgcc -static-libstdc++ -static")
    set(CMAKE_EXE_LINKER_FLAGS=-static-libgcc -static-libstdc++ -static)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -s -O3")
else()
    set(CompilerFlags
            CMAKE_CXX_FLAGS
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_C_FLAGS
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELEASE
        )
     foreach(CompilerFlag ${CompilerFlags})
            string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
     endforeach()
     message(STATUS "Compile type: STATIC")
endif()

# c++ standard for optimum release 
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
#set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# src and incs based on win or linux
set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})  
set(DATA_COLLECTOR_INCS 
  ${PROJECT_ROOT}/include/data_collector
  ${PROJECT_ROOT}/include/data_transmitter
  ${PROJECT_ROOT}/include/utils
  )
set(DATA_COLLECTOR_SRCS 
  ${PROJECT_ROOT}/src/data_collector/ConcreteActivities.cpp
  ${PROJECT_ROOT}/src/data_collector/ConcreteActivityRecognition.cpp
  ${PROJECT_ROOT}/src/data_collector/ConcreteDeviceInformation.cpp
  ${PROJECT_ROOT}/src/data_collector/ConcreteSystemInformation.cpp
  ${PROJECT_ROOT}/src/data_collector/DataCollector.cpp
  ${PROJECT_ROOT}/src/data_collector/TaskPool.cpp
  ${PROJECT_ROOT}/src/data_collector/CpuInfo.cpp
  ${PROJECT_ROOT}/src/data_transmitter/DataTransmitter.cpp
  ${PROJECT_ROOT}/src/utils/Client.cpp
  )

if(CMAKE_HOST_WIN32)
  set(FILE_WATCHER_INCS
    ${PROJECT_ROOT}/include/data_collector/FileWatcher.h
  )
  add_executable(
    FileWatcher
    ${PROJECT_ROOT}/src/data_collector/FileWatcher.cpp
    ${FILE_WATCHER_INCS}	
  )
  target_include_directories(
    FileWatcher 
    PUBLIC
    ${PROJECT_BINARY_DIR}
  )
  message(STATUS "Compile file watcher")
endif()

add_executable(
  demo
  ${PROJECT_ROOT}/src/main.cpp
  ${DATA_COLLECTOR_SRCS} 
  ${DATA_COLLECTOR_INCS}	
)

target_include_directories(
  demo 
  PUBLIC
  ${PROJECT_BINARY_DIR}
  ${DATA_COLLECTOR_INCS}
)

# install path
set(CPACK_ALL_INSTALL_TYPES Full) 
if(CMAKE_HOST_WIN32)
    install(TARGETS demo FileWatcher DESTINATION bin COMPONENT FileWatcher)
    #set(CPACK_COMPONENT_BINARIES_INSTALL_TYPES Full Upgrade)
    #set(CPACK_COMPONENT_DATA_FILES_INSTALL_TYPES Full)
    #set(CPACK_COMPONENTS_ALL binaries)
    message(STATUS "System is: WINDOWS")
elseif(CMAKE_HOST_UNIX)
    install(TARGETS demo 
	    RUNTIME DESTINATION "~/Downloads/demo"
   	  LIBRARY DESTINATION "~/Downloads/demo"
    	DESTINATION "~/Downloads/demo"
    )
    set(CPACK_GENERATOR "DEB")
    set(CPACK_PACKAGE_NAME "demo")
    set(CPACK_DEBIAN_PACKAGE_NAME "demo")
    #set(CPACK_DEBIAN_PACKAGE_DEPENDS "")
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "Optional")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Data collector proxy is a free software displaying system information.")
    set(CPACK_PACKAGE_CONTACT "xian.zhangs@aliyun.com")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "ZhangXian")
    message(STATUS "System is: Linux")
else()
    message(STATUS "System is: Unknown System")
endif()


if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
    message(STATUS "System OS: 64 bit")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "x86")
    message(STATUS "System OS: 32 bit")
else()
    message(STATUS "System OS: ARM")
endif()

include(InstallRequiredSystemLibraries)

set(_VERSION_MAJOR 1)
set(_VERSION_MINOR 0)
set(_VERSION_PATCH 0)
set(CPACK_PACKAGE_VERSION_MAJOR "${_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${_VERSION_PATCH}")
set(CPACK_SOURCE_GENERATOR "ZIP")
set(CPACK_SOURCE_IGNORE_FILES
        ${PROJECT_SOURCE_DIR}/build
        ${PROJECT_SOURCE_DIR}/.git
        ${PROJECT_SOURCE_DIR}/.gitignore
        ${PROJECT_SOURCE_DIR}/.vscode)	
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")

include(CPack)
