#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <mutex>


#define MAX_LEN 200
#define NUM_COLORS 6

using namespace std;

class ChatServer {
public:
    // Constructor to initialize the chat server with a given port
    ChatServer(int port);

    // Method to start the chat server
    void start();

private:
    // Struct to represent a connected terminal (client)
    struct Terminal {
        int id;
        string name;
        int socket;
        thread th;
    };

    vector<Terminal> clients;   // List of connected clients
    string def_col;             // Default color code
    string colors[NUM_COLORS];  // Color codes for different clients
    int seed;                   // Seed for generating client IDs
    mutex cout_mtx, clients_mtx;// Mutex for thread safety
    int server_socket;          // Server socket descriptor
    int port;                   // Port on which the server listens

    // Method to get color code based on client ID
    string color(int code);

    // Method to set the name of a client
    void set_name(int id, const char* name);

    // Thread-safe method to print shared messages
    void shared_print(const string& str, bool endLine = true);

    // Method to broadcast message to all clients except the sender
    void broadcast_message(const string& message, int sender_id);

    // Method to broadcast a number to all clients except the sender
    void broadcast_message(int num, int sender_id);

    // Method to end the connection with a client
    void end_connection(int id);

    // Method to handle client communication
    void handle_client(int client_socket, int id);
};

#endif // CHATSERVER_H
