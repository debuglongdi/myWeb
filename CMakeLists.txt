cmake_minimum_required(VERSION 3.16)
project(mymuduo)

# 设置调试信息 以及启动c++11语言标准
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -Wall -std=c++11")

#如果想要DEBUG的输出，需要定义MYMUDUO_DEBUG宏
# add_compile_definitions(MYMUDUO_DEBUG)

# 添加包含文件，为了使得我们编写的 .cpp 源文件能够使用在不同目录里的包含文件
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

#设置动态库输出文件
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)




# 生成库文件mymuduo_utils，第一个参数是我们希望生成的库的名字，后续是生成这个库所需要的所有源文件
add_library(mymuduo_utils SHARED
        src/utils/Logger.cc 
        src/utils/Timestamp.cc
        src/utils/CurrentThread.cc
        )

# 生成库文件mymuduo_net，第一个参数是我们希望生成的库的名字，后续是生成这个库所需要的所有源文件
add_library(mymuduo_net SHARED
        src/net/SockOps.cc
        src/net/InetAddress.cc
        src/net/Channel.cc 
        src/net/Poller.cc
        src/net/EpollPoller.cc
        src/net/EventLoop.cc
        src/net/DefaultPoller.cc
        src/net/Thread.cc
        src/net/EventLoopThread.cc
        src/net/EventLoopThreadPool.cc
        src/net/Socket.cc
        src/net/Acceptor.cc
        src/net/TcpServer.cc
        src/net/TcpConnection.cc
        src/net/Buffer.cc
        src/net/Connector.cc
        src/net/TcpClient.cc
        )

#########################################################################################
#########################################################################################

#生成第一个可执行文件
# add_executable(Logger_test src/test/Logger_test.cc)
# target_link_libraries(Logger_test mymuduo_utils)

# #生成第二个可执行文件
# add_executable(InetAddress_test src/test/InetAddress_test.cc)
# target_link_libraries(InetAddress_test mymuduo_net mymuduo_utils)



#########################################################################################
#########################################################################################
