rem Need MinGW

g++ example/client.cpp -std=c++1z -g3 -ggdb -O3 -DWIN32 -Isrc/include -o bin/client
g++ example/server.cpp -std=c++1z -g3 -ggdb -O3 -DWIN32 -Isrc/include -o bin/server