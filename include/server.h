#ifndef SERVER_H
#define SERVER_H

#include "socket.h"
#include <iostream>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <vector>
#include <fstream>

#define QUEUE 1
#define PORT 389   
#define BUFFER 256 

class Server {
    private:
        int port;
        std::string file;
        int newsocket;
        // Socket socket;
        // int newsocket;
        // int len, msg_size, i;
        void checkExistenceOfFile(std::string file);
        //static void sigintHandler(int signal); TODO
    public:
        Server();
        void parseArguments(int argc, char* argv[]);
        void printUsage();
        int run();

};

#endif