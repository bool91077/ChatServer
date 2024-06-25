#include <iostream>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <signal.h>
#include <cstring>
#include <vector>
#include <algorithm> // 需要包含这个头文件

#define DB_SERVER_PORT 5558

class DatabaseServer {
public:
    DatabaseServer() : base(nullptr), listener(nullptr) {}

    void start(int port) {
        // 初始化 libevent 的 event_base
        base = event_base_new();
        if (!base) {
            std::cerr << "无法初始化 libevent!" << std::endl;
            return;
        }

        // 设置 sockaddr_in 结构体，用于 IPv4 地址
        struct sockaddr_in sin;
        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = htonl(0); // 监听所有可用接口
        sin.sin_port = htons(port);

        // 创建一个新的监听器来接受连接
        listener = evconnlistener_new_bind(base, accept_conn_cb, (void *)this,
                                           LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
                                           (struct sockaddr *) &sin, sizeof(sin));

        if (!listener) {
            perror("无法创建监听器!");
            return;
        }

        // 设置信号处理器，处理 SIGINT (Ctrl+C)
        struct event *signal_event;
        signal_event = evsignal_new(base, SIGINT, signal_cb, (void *) base);

        if (!signal_event || event_add(signal_event, nullptr) < 0) {
            std::cerr << "无法创建或添加信号事件!" << std::endl;
            return;
        }

        std::cout << "数据库服务器启动，监听端口 " << port << std::endl;

        // 进入 libevent 的事件循环
        event_base_dispatch(base);

        // 事件循环结束后清理资源
        evconnlistener_free(listener);
        event_free(signal_event);
        event_base_free(base);

        std::cout << "数据库服务器端口 " << port << " 关闭。" << std::endl;
    }

private:
    struct event_base *base;                     // libevent 的事件基础
    struct evconnlistener *listener;             // 接受连接的监听器
    std::vector<struct bufferevent *> clients;   // 已连接客户端的 bufferevent 列表

    // SIGINT (Ctrl+C) 信号处理器
    static void signal_cb(evutil_socket_t sig, short events, void *user_data) {
        struct event_base *base = static_cast<struct event_base *>(user_data);
        struct timeval delay = {2, 0};
        std::cout << "捕获到中断信号; 将在两秒内优雅退出。" << std::endl;
        event_base_loopexit(base, &delay);
    }

    // 接受连接的回调函数
    static void accept_conn_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address,
                               int socklen, void *ctx) {
        auto *server = static_cast<DatabaseServer *>(ctx);
        server->accept_connection(listener, fd, address, socklen);
    }

    // 处理接受的连接
    void accept_connection(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address,
                           int socklen) {
        // 创建一个新的 bufferevent 处理与客户端的通信
        auto *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
        bufferevent_setcb(bev, read_cb, nullptr, error_cb, this); // 设置回调函数
        bufferevent_enable(bev, EV_READ | EV_WRITE); // 启用读写事件
        clients.push_back(bev); // 将新的 bufferevent 添加到客户端列表
    }

    // 读取数据的回调函数
    static void read_cb(struct bufferevent *bev, void *ctx) {
        auto *server = static_cast<DatabaseServer *>(ctx);
        server->handle_read(bev);
    }

    // 处理读取事件
    void handle_read(struct bufferevent *bev) {
        // 模拟数据库查询处理
        const char *query_result = "查询结果: 来自数据库服务器的问候!\n";

        // 将查询结果发送回客户端
        bufferevent_write(bev, query_result, strlen(query_result));
    }

    // 错误处理的回调函数
    static void error_cb(struct bufferevent *bev, short error, void *ctx) {
        auto *server = static_cast<DatabaseServer *>(ctx);
        server->handle_error(bev, error);
    }

    // 处理连接错误
    void handle_error(struct bufferevent *bev, short error) {
        // 处理不同类型的错误
        if (error & BEV_EVENT_EOF) {
            std::cout << "连接关闭。" << std::endl;
        } else if (error & BEV_EVENT_ERROR) {
            std::cerr << "连接发生错误: " << strerror(errno) << std::endl;
        } else if (error & BEV_EVENT_TIMEOUT) {
            std::cout << "连接超时。" << std::endl;
        }

        // 从客户端列表中移除 bufferevent 并释放资源
        auto it = std::find(clients.begin(), clients.end(), bev);
        if (it != clients.end()) {
            clients.erase(it);
        }

        bufferevent_free(bev);
    }
};

// 程序入口
int main() {
    DatabaseServer dbServer;
    dbServer.start(DB_SERVER_PORT); // 启动数据库服务器

    return 0;
}
