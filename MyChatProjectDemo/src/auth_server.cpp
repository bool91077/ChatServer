#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <event2/buffer.h> // 添加这个头文件，用于缓冲区操作
#include <iostream>
#include <cstring>
#include <unordered_map>

#define LISTEN_PORT 5001 // 定义监听端口为5001

// 定义一个用户和密码的映射，key是用户名，value是密码
std::unordered_map<std::string, std::string> users{
    {"user1", "password1"},
    {"user2", "password2"}
};

// 读取回调函数，当有数据可读时调用
void read_cb(struct bufferevent *bev, void *ctx) {
    // 获取输入和输出缓冲区
    struct evbuffer *input = bufferevent_get_input(bev);
    struct evbuffer *output = bufferevent_get_output(bev);
    char buf[1024]; // 定义一个缓冲区，用于存储读取的数据
    int n = evbuffer_remove(input, buf, sizeof(buf) - 1); // 从输入缓冲区读取数据
    buf[n] = '\0'; // 将读取的数据转换为字符串，并添加字符串结束符

    std::string response; // 用于存储响应信息
    std::string request(buf); // 将读取的数据转换为std::string

    // 检查请求是否包含"validate"字符串
    if (request.find("validate") != std::string::npos) {
        // 提取用户名和密码
        std::string user = request.substr(9); // 获取"validate"之后的部分
        std::string username = user.substr(0, user.find(':')); // 获取用户名
        std::string password = user.substr(user.find(':') + 1); // 获取密码

        // 验证用户名和密码是否匹配
        if (users[username] == password) {
            response = "{\"status\":\"success\"}"; // 如果匹配，返回成功状态
        } else {
            response = "{\"status\":\"fail\"}"; // 如果不匹配，返回失败状态
        }
    } else {
        response = "{\"status\":\"unknown\"}"; // 如果请求格式不对，返回未知状态
    }

    // 将响应信息添加到输出缓冲区
    evbuffer_add(output, response.c_str(), response.size());
}

// 事件回调函数，当连接发生错误或结束时调用
void event_cb(struct bufferevent *bev, short events, void *ctx) {
    if (events & BEV_EVENT_ERROR) {
        std::cerr << "Error from bufferevent" << std::endl; // 如果发生错误，输出错误信息
    }
    if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        bufferevent_free(bev); // 如果连接结束或发生错误，释放bufferevent
    }
}

// 接受连接回调函数，当有新的客户端连接时调用
void accept_conn_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *addr, int len, void *ctx) {
    struct event_base *base = evconnlistener_get_base(listener); // 获取事件基础
    struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE); // 为新连接创建一个bufferevent
    bufferevent_setcb(bev, read_cb, nullptr, event_cb, nullptr); // 设置bufferevent的回调函数
    bufferevent_enable(bev, EV_READ | EV_WRITE); // 启用读写事件
}

// 接受错误回调函数，当监听器发生错误时调用
void accept_error_cb(struct evconnlistener *listener, void *ctx) {
    struct event_base *base = evconnlistener_get_base(listener); // 获取事件基础
    int err = EVUTIL_SOCKET_ERROR(); // 获取当前的socket错误码
    std::cerr << "Error (" << err << "): " << evutil_socket_error_to_string(err) << std::endl; // 输出错误信息
    event_base_loopexit(base, nullptr); // 退出事件循环
}

int main() {
    struct event_base *base; // 定义事件基础
    struct evconnlistener *listener; // 定义连接监听器
    struct sockaddr_in sin; // 定义地址结构体

    base = event_base_new(); // 创建一个新的事件基础
    if (!base) {
        std::cerr << "Could not initialize libevent!" << std::endl; // 如果创建失败，输出错误信息
        return 1;
    }

    memset(&sin, 0, sizeof(sin)); // 将地址结构体清零
    sin.sin_family = AF_INET; // 设置地址族为IPv4
    sin.sin_addr.s_addr = htonl(0); // 设置IP地址，0表示监听所有IP地址
    sin.sin_port = htons(LISTEN_PORT); // 设置端口号

    // 创建并绑定一个新的连接监听器
    listener = evconnlistener_new_bind(base, accept_conn_cb, nullptr,
                                       LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
                                       (struct sockaddr *)&sin, sizeof(sin));

    if (!listener) {
        std::cerr << "Could not create a listener!" << std::endl; // 如果创建失败，输出错误信息
        return 1;
    }
    evconnlistener_set_error_cb(listener, accept_error_cb); // 设置监听器的错误回调函数

    event_base_dispatch(base); // 进入事件循环
    evconnlistener_free(listener); // 释放监听器
    event_base_free(base); // 释放事件基础

    return 0;
}
