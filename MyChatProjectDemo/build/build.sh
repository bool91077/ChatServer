#!/bin/bash

set -e

# 编译 auth_server.cpp
echo "编译 auth_server..."
g++ -o auth_server auth_server.cpp -levent -lpthread
echo "编译 auth_server 成功!"

# 编译 chat_server.cpp
echo "编译 chat_server..."
g++ -o chat_server chat_server.cpp -levent -lpthread
echo "编译 chat_server 成功!"

# 编译 client_app.cpp
echo "编译 client_app..."
g++ -o client_app client_app.cpp -levent -lpthread
echo "编译 client_app 成功!"

# 编译 db_server.cpp
echo "编译 db_server..."
g++ -o db_server db_server.cpp -levent -lpthread
echo "编译 db_server 成功!"

# 编译 gateway_server.cpp
echo "编译 gateway_server..."
g++ -o gateway_server gateway_server.cpp -levent -lpthread
echo "编译 gateway_server 成功!"

# 编译 log_server.cpp
echo "编译 log_server..."
g++ -o log_server log_server.cpp -levent -lpthread
echo "编译 log_server 成功!"
