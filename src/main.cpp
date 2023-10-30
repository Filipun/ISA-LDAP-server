/**
     * @file main.cpp
     * @author Filip Polomski
     * 
     * @brief 
     */


#include "../include/server.h"

int main(int argc, char* argv[])
{
    Server server;
    server.parseArguments(argc, argv);
    server.run(); 

    return 0;
}