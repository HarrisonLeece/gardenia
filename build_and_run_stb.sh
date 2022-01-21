#!/bin/bash
g++ -I /$(pwd)/libs -L /$(pwd)/libs  -ggdb -g3 -Wall -o gardenia_stb ./main_stb.cpp -lstdc++fs -std=c++17  `pkg-config opencv4 --cflags --libs` -pthread
./gardenia_stb
