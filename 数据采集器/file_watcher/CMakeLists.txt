#1 版本
cmake_minimum_required(VERSION 3.0)

#2 项目名称和版本
project(FileWatcher)

#3 判断平台
if (CMAKE_HOST_WIN32)
    message(STATUS "System is: WINDOWS")
else(CMAKE_HOST_UNIX)
    set(LINUX 1)
    message(STATUS "System is: Linux")
endif()

#4 静态编译
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

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
   message(STATUS "System OS: 64 bit")
   set(ADDRESS_MODEL 64)
   set(NODE_TARGET x64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
   message(STATUS "System OS: 32 bit")
   set(ADDRESS_MODEL 32)
   set(NODE_TARGET x86)
else()
    message(STATUS "System OS: ARM")
endif()

#5 设置编译器标准和应用大小
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
#set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

#6 定义项目目录
set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

#7 添加子目录
FILE(GLOB_RECURSE CURRENT_INCLUDE include/*.h)
FILE(GLOB_RECURSE CURRENT_SOURCES src/*.cpp) 


#8 生成应用
add_executable(
    FileWatcher
    ${PROJECT_ROOT}/src/main.cpp
    ${CURRENT_SOURCES} 
    ${CURRENT_INCLUDE}	
)

#9 引用头文件
target_include_directories(FileWatcher PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          )

#10 应用安装位置
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/build/)

#11 cpack 设置
#install(TARGETS FileWatcher DESTINATION bin COMPONENT binaries)
install(TARGETS FileWatcher DESTINATION bin)
set(CPACK_ALL_INSTALL_TYPES Full) 
#set(CPACK_COMPONENT_BINARIES_INSTALL_TYPES Full Upgrade)
#set(CPACK_COMPONENT_DATA_FILES_INSTALL_TYPES Full)
#set(CPACK_COMPONENTS_ALL binaries)

include(InstallRequiredSystemLibraries)
include(CPack)
