#include <iostream>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/util.h>
#include <cstring>
#include <thread>
#include <atomic>
#include <event2/buffer.h> // 添加这个头文件，用于缓冲区操作

#define SERVER_ADDR "127.0.0.1" // 定义服务器地址
#define SERVER_PORT 5555 // 定义服务器端口

std::atomic<bool> running(true); // 定义一个原子布尔变量，用于控制程序的运行状态

// 读取回调函数，当有数据可读时调用
void read_cb(struct bufferevent *bev, void *ctx) {
    struct evbuffer *input = bufferevent_get_input(bev);
    char buf[1024];
    size_t n;

    // 不断读取输入缓冲区的数据
    while ((n = evbuffer_remove(input, buf, sizeof(buf))) > 0) {
        buf[n] = '\0'; // 将读取的数据转换为字符串，并添加字符串结束符
        std::cout << "从服务器收到: " << buf; // 输出从服务器接收到的数据
    }
}

// 事件回调函数，当连接发生特定事件时调用
void event_cb(struct bufferevent *bev, short events, void *ctx) {
    if (events & BEV_EVENT_CONNECTED) {
        std::cout << "已连接到服务器。" << std::endl; // 如果连接成功，输出成功信息
    } else if (events & BEV_EVENT_ERROR) {
        std::cerr << "连接服务器时出错。" << std::endl; // 如果发生错误，输出错误信息
        bufferevent_free(bev); // 释放bufferevent
        running = false; // 设置运行状态为false，停止程序
    } else if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        std::cout << "连接已关闭。" << std::endl; // 如果连接关闭或发生错误，输出信息
        bufferevent_free(bev); // 释放bufferevent
        running = false; // 设置运行状态为false，停止程序
    }
}

// 用户输入线程函数，处理用户输入并发送到服务器
void user_input_thread(struct bufferevent *bev) {
    std::string line;
    // 不断读取用户输入
    while (running && std::getline(std::cin, line)) {
        if (!running) break; // 如果程序已停止，退出循环
        line += "\n"; // 添加换行符
        bufferevent_write(bev, line.c_str(), line.size()); // 将用户输入发送到服务器
    }
}

int main() {
    struct event_base *base;
    struct bufferevent *bev;
    struct sockaddr_in sin;

    base = event_base_new(); // 创建一个新的事件基础
    if (!base) {
        std::cerr << "无法初始化libevent！" << std::endl; // 如果创建失败，输出错误信息
        return 1;
    }

    memset(&sin, 0, sizeof(sin)); // 将地址结构体清零
    sin.sin_family = AF_INET; // 设置地址族为IPv4
    sin.sin_port = htons(SERVER_PORT); // 设置端口号

    // 将服务器地址转换为网络格式
    if (evutil_inet_pton(AF_INET, SERVER_ADDR, &sin.sin_addr) <= 0) {
        std::cerr << "无效的服务器地址" << std::endl; // 如果地址无效，输出错误信息
        return 1;
    }

    bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE); // 创建一个新的bufferevent
    bufferevent_setcb(bev, read_cb, nullptr, event_cb, nullptr); // 设置bufferevent的回调函数
    bufferevent_enable(bev, EV_READ | EV_WRITE); // 启用读写事件

    // 连接到服务器
    if (bufferevent_socket_connect(bev, reinterpret_cast<struct sockaddr *>(&sin), sizeof(sin)) < 0) {
        std::cerr << "连接服务器失败" << std::endl; // 如果连接失败，输出错误信息
        bufferevent_free(bev); // 释放bufferevent
        return 1;
    }

    // 创建一个新的线程，用于处理用户输入
    std::thread input_thread(user_input_thread, bev);
    event_base_dispatch(base); // 进入事件循环
    running = false; // 设置运行状态为false，停止程序
    input_thread.join(); // 等待用户输入线程结束
    event_base_free(base); // 释放事件基础

    return 0;
}
