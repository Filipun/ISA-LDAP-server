/**
 * @file socket.cpp
 * @author Filip Polomski, xpolom00
 * @brief Socket file with implementation of Socket class
 * @version 0.1
 * @date 2023-11-20
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <iostream>
#include "../include/socket.h"

/**
 * @brief Construct a new Socket:: Socket object
 * 
 * @param port 
 */
Socket::Socket(int port)
{
    this->port = port;
    this->optValue = 0;
    this->fd = 0;
}

/**
 * @brief Method for creating socket
 * 
 */
void Socket::createSocket()
{
    if ((this->fd = socket(AF_INET6, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "Socket(): could not create the socket\n");
        exit(1);
    }
    if (setsockopt(this->fd, IPPROTO_IPV6, IPV6_V6ONLY, &this->optValue, sizeof(&this->optValue)) == -1)
    {
        printf("Socket setting for dual-stack failed. Server will run only on IPv6.\n");
    }
    printf("* Socket succesfully created\n");
}

/**
 * @brief Method for binding socket
 * 
 */
void Socket::bindSocket(sockaddr_in6 server)
{
    if (bind(this->fd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        fprintf(stderr, "bind() failed\n");
        exit(1);
    }
    printf("* Socket succesfully bound to the port using bind()\n");
}

/**
 * @brief Method getting socket
 * 
 */
int Socket::getSocket()
{
    return this->fd;
}