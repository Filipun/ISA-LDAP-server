/**
    * @file main.cpp
    * @author Filip Polomski, xpolom00
    * @brief Main file for ISA LDAPv2 server
    * @version 1.0
    * @date 2023-20-11
    * 
    * @copyright Copyright (c) 2023
    * 
 */


#include "../include/server.h"

/**
 * @brief Main function
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char* argv[])
{
    Server server;
    server.parseArguments(argc, argv);
    server.run(); 

    return 0;
}
