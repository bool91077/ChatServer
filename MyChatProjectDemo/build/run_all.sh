#!/bin/bash

# Execute auth_server
echo "Running auth_server..."
./auth_server &
sleep 1

# Execute chat_server
echo "Running chat_server..."
./chat_server &
sleep 1

# Execute client_app
echo "Running client_app..."
./client_app &
sleep 1

# Execute db_server
echo "Running db_server..."
./db_server &
sleep 1

# Execute gateway_server
echo "Running gateway_server..."
./gateway_server &
sleep 1

# Execute log_server
echo "Running log_server..."
./log_server &
sleep 1

# Wait for user interruption
echo "All servers started. Press Ctrl+C to stop."

# Trap Ctrl+C to clean up
trap "echo 'Stopping servers...'; pkill -P $$; exit" SIGINT

# Keep the script running until interrupted
while :
do
    sleep 1
done
