#include <iostream>
#include "../include/socket.h"

Socket::Socket(int port)
{
    this->port = port;
    this->optValue = 0;
    this->fd = 0;
}

// 
void Socket::createSocket()
{
    if ((this->fd = socket(AF_INET6, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "Socket(): could not create the socket\n");
        exit(1);
    }
    if (setsockopt(this->fd, IPPROTO_IPV6, IPV6_V6ONLY, &this->optValue, sizeof(&this->optValue)) == -1)
    {
        fprintf(stderr, "Socket setting for dual-stack failed. Server will run only on IPv6.\n");
        //exit(1);
    }
    printf("* Socket succesfully created\n");
}

void Socket::bindSocket(sockaddr_in6 server)
{
    if (bind(this->fd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        fprintf(stderr, "bind() failed\n");
        exit(1);
    }
    printf("* Socket succesfully bound to the port using bind()\n");
}

int Socket::getSocket()
{
    return this->fd;
}