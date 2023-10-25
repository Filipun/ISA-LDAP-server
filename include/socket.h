#ifndef SOCKET_H
#define SOCKET_H

#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <err.h>
#include <ctype.h>


class Socket {
    private:
        int port;
        int optValue;
        
    public:
        Socket(int port);
        int fd;
        void createSocket();
        void setSocket();
        void bindSocket(sockaddr_in6 server);
        int getSocket();
};


//TODO LDAP server





#endif