#!/bin/bash

g++ -o client client.cpp -g -w
g++ -W -o server go_server.cpp -g $(pkg-config --cflags --libs libmongoc-1.0) -I/opt/ssl/include/ -L/opt/ssl/lib/ -lcrypto -w
g++ -W -o prac prac_server.cpp -g $(pkg-config --cflags --libs libmongoc-1.0) -I/opt/ssl/include/ -L/opt/ssl/lib/ -lcrypto -w
#$(pkg-config --cflags --libs libbson-1.0 libmongoc-1.0)
