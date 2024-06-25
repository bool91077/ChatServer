#include "src/ChatServer.h"

int main() {

    ChatServer server(10000);
    server.start();
    return 0;
}