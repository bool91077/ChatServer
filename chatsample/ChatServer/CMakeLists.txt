cmake_minimum_required(VERSION 3.10)

# 设置项目名称和版本
project(ChatApplication VERSION 1.0)

# 指定 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 包含头文件目录
include_directories(${CMAKE_SOURCE_DIR})

# 查找线程库
find_package(Threads REQUIRED)

# 添加可执行文件并包含源文件路径
add_executable(server 
    ./src/ChatServer.cpp
    main.cpp
    # 添加其他源文件...
)
target_link_libraries(server PRIVATE ${CMAKE_THREAD_LIBS_INIT})

# 添加可执行文件 client
# add_executable(client client.cpp Logger.cpp)
# target_link_libraries(client PRIVATE ${CMAKE_THREAD_LIBS_INIT})

