#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <signal.h>
#include <mutex>

#define MAX_LEN 200
#define NUM_COLORS 6

using namespace std;

class ChatClient {
public:
    ChatClient(const string &server_ip, int server_port);
    void start();
    void stop();

private:
    int client_socket;
    bool exit_flag = false;
    thread t_send, t_recv;
    string def_col = "\033[0m";
    string colors[NUM_COLORS] = {"\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m"};
    
    static void catch_ctrl_c(int signal);
    string color(int code);
    void eraseText(int cnt);
    void send_message();
    void recv_message();
    
    static ChatClient *instance; // For handling Ctrl+C
    static mutex cout_mtx; // For synchronizing cout statements
};

ChatClient* ChatClient::instance = nullptr;
mutex ChatClient::cout_mtx;

ChatClient::ChatClient(const string &server_ip, int server_port) {
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in client;
    client.sin_family = AF_INET;
    client.sin_port = htons(server_port);
    client.sin_addr.s_addr = inet_addr(server_ip.c_str());
    memset(&client.sin_zero, 0, sizeof(client.sin_zero));

    if (connect(client_socket, (struct sockaddr *)&client, sizeof(struct sockaddr_in)) == -1) {
        perror("connect");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, catch_ctrl_c);
    instance = this; // Set the instance for static function
}

void ChatClient::start() {
    char name[MAX_LEN];
    cout << "输入你的姓名 : ";
    cin.getline(name, MAX_LEN);
    send(client_socket, name, sizeof(name), 0);

    cout << colors[NUM_COLORS - 1] << "\n\t  ====== 欢迎加入聊天室 ======   " << endl
         << def_col;

    t_send = thread(&ChatClient::send_message, this);
    t_recv = thread(&ChatClient::recv_message, this);

    if (t_send.joinable())
        t_send.join();
    if (t_recv.joinable())
        t_recv.join();
}

void ChatClient::stop() {
    exit_flag = true;
    if (t_send.joinable()) {
        t_send.detach();
    }
    if (t_recv.joinable()) {
        t_recv.detach();
    }
    close(client_socket);
}

void ChatClient::catch_ctrl_c(int signal) {
    if (instance) {
        char str[MAX_LEN] = "#exit";
        send(instance->client_socket, str, sizeof(str), 0);
        instance->stop();
        exit(signal);
    }
}

string ChatClient::color(int code) {
    return colors[code % NUM_COLORS];
}

void ChatClient::eraseText(int cnt) {
    lock_guard<mutex> guard(cout_mtx);
    for (int i = 0; i < cnt; i++) {
        cout << "\b \b";
    }
    cout.flush();
}

void ChatClient::send_message() {
    while (true) {
        cout << colors[1] << "你 : " << def_col;
        char str[MAX_LEN];
        cin.getline(str, MAX_LEN);
        send(client_socket, str, sizeof(str), 0);
        if (strcmp(str, "#exit") == 0) {
            stop();
            break;
        }
    }
}

void ChatClient::recv_message() {
    while (!exit_flag) {
        char name[MAX_LEN], str[MAX_LEN];
        int color_code;
        int bytes_received = recv(client_socket, name, sizeof(name), 0);
        if (bytes_received <= 0) {
            continue;
        }
        recv(client_socket, &color_code, sizeof(color_code), 0);
        recv(client_socket, str, sizeof(str), 0);
        eraseText(6);
        lock_guard<mutex> guard(cout_mtx);
        if (strcmp(name, "#NULL") != 0) {
            cout << color(color_code) << name << " : " << def_col << str << endl;
        } else {
            cout << color(color_code) << str << endl;
        }
        cout << colors[1] << "你 : " << def_col;
        cout.flush();
    }
}

int main() {
    string server_ip = "127.0.0.1"; // Change this to the server's IP address
    int server_port = 10000; // Change this to the server's port

    try {
        ChatClient client(server_ip, server_port);
        client.start();
    } catch (const exception &e) {
        cerr << "Error: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    return 0;
}
