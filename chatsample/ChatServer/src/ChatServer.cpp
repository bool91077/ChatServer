#include "ChatServer.h"

// Constructor to initialize the chat server with a given port
ChatServer::ChatServer(int port) : port(port), def_col("\033[0m"), seed(0) {
    colors[0] = "\033[31m";
    colors[1] = "\033[32m";
    colors[2] = "\033[33m";
    colors[3] = "\033[34m";
    colors[4] = "\033[35m";
    colors[5] = "\033[36m";
}

// Method to start the chat server
void ChatServer::start() {
    // Initialize socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket: ");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;
    memset(&server.sin_zero, 0, sizeof(server.sin_zero));

    // Bind socket
    if (bind(server_socket, (sockaddr*)&server, sizeof(sockaddr_in)) == -1) {
        perror("bind error: ");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_socket, 8) == -1) {
        perror("listen error: ");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    cout << colors[NUM_COLORS - 1] << "\n\t  ====== 欢迎加入聊天室 ======   " << endl << def_col;

    sockaddr_in client;
    unsigned int len = sizeof(sockaddr_in);
    int client_socket;

    while (true) {
        if ((client_socket = accept(server_socket, (sockaddr*)&client, &len)) == -1) {
            perror("accept error: ");
            continue;
        }
        seed++;
        thread t(&ChatServer::handle_client, this, client_socket, seed);
        lock_guard<mutex> guard(clients_mtx);
        clients.push_back({seed, "Anonymous", client_socket, move(t)});
    }

    for (auto& client : clients) {
        if (client.th.joinable())
            client.th.join();
    }

    close(server_socket);
}

// Method to get color code based on client ID
string ChatServer::color(int code) {
    return colors[code % NUM_COLORS];
}

// Method to set the name of a client
void ChatServer::set_name(int id, const char* name) {
    lock_guard<mutex> guard(clients_mtx);
    for (auto& client : clients) {
        if (client.id == id) {
            client.name = name;
            break;
        }
    }
}

// Thread-safe method to print shared messages
void ChatServer::shared_print(const string& str, bool endLine) {
    lock_guard<mutex> guard(cout_mtx);
    cout << str;
    if (endLine) {
        cout << endl;
    }
}

// Method to broadcast message to all clients except the sender
void ChatServer::broadcast_message(const string& message, int sender_id) {
    lock_guard<mutex> guard(clients_mtx);
    for (const auto& client : clients) {
        if (client.id != sender_id) {
            send(client.socket, message.c_str(), message.length() + 1, 0);
        }
    }
}

// Method to broadcast a number to all clients except the sender
void ChatServer::broadcast_message(int num, int sender_id) {
    lock_guard<mutex> guard(clients_mtx);
    for (const auto& client : clients) {
        if (client.id != sender_id) {
            send(client.socket, &num, sizeof(num), 0);
        }
    }
}

// Method to end the connection with a client
void ChatServer::end_connection(int id) {
    lock_guard<mutex> guard(clients_mtx);
    auto it = find_if(clients.begin(), clients.end(), [id](const Terminal& client) {
        return client.id == id;
    });
    if (it != clients.end()) {
        it->th.detach();
        close(it->socket);
        clients.erase(it);
    }
}

// Method to handle client communication
void ChatServer::handle_client(int client_socket, int id) {
    char name[MAX_LEN], str[MAX_LEN];
    recv(client_socket, name, sizeof(name), 0);
    set_name(id, name);

    string welcome_message = string(name) + " 加入";
    broadcast_message("#NULL", id);
    broadcast_message(id, id);
    broadcast_message(welcome_message, id);
    shared_print(color(id) + welcome_message + def_col);

    while (true) {
        int bytes_received = recv(client_socket, str, sizeof(str), 0);
        if (bytes_received <= 0) {
            break;
        }
        if (strcmp(str, "#exit") == 0) {
            string message = string(name) + " 离开";
            broadcast_message("#NULL", id);
            broadcast_message(id, id);
            broadcast_message(message, id);
            shared_print(color(id) + message + def_col);
            end_connection(id);
            return;
        }
        broadcast_message(string(name), id);
        broadcast_message(id, id);
        broadcast_message(string(str), id);
        shared_print(color(id) + name + " : " + def_col + str);
    }
    end_connection(id);
}
