#ifndef SERVER_H
#define SERVER_H

#include "socket.h"
#include <iostream>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include<signal.h>

#define QUEUE 1    
#define PORT 389   
#define BUFFER 256 

class Server {
    private:
        int port;
        std::string file;
        // Socket socket;
        // int newsocket;
        // int len, msg_size, i;
    public:
        Server();
        void parseArguments(int argc, char* argv[]);
        void printUsage();
        int run();

};

#endif