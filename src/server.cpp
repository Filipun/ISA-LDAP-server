/**
 * @file server.cpp
 * @author Filip Polomski, xpolom00
 * @brief Server file with implementation of TCP server class
 * @version 1.0
 * @date 2023-11-20
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "../include/server.h"
#include "../include/ldapParser.h"
#include "../include/LDAP.h"

/**
 * @brief Construct a new Server:: Server object
 * 
 */
Server::Server()
{
    this->port = PORT;
    this->file = "";
}

/**
 * @brief Method for parsing arguments
 * 
 * @param args Number of arguments
 * @param argv Array of arguments
 */
void Server::parseArguments(int args, char* argv[])
{
    int c;

    while (1)
    {
        static option long_options[] = {
            {"help",    no_argument, 0, 'h'},
            {"port",    required_argument, 0, 'p'},
            {"file",    required_argument, 0, 'f'}
        };

        c = getopt_long(args, argv, "hp:f:", long_options, nullptr);

        if (c == -1){
            break;
        }

        switch(c)
        {
            case 'h':
                this->printUsage();
                break;
            case 'p':
                this->port = atoi(optarg);
                break;
            case 'f':
                this->checkExistenceOfFile(optarg);
                this->file = optarg;
                break;
            default:
                fprintf(stderr, "Uknown argument!\nUse -h or --help to se available arguments\n");
                exit(1);
        }
    }
}

/**
 * @brief Method for running server and listening for incoming connections
 * 
 * @return int 
 */
int Server::run()
{
    int newsocket;
    int len;
    pid_t pid; long p;      // process ID number (PID)
    struct sigaction sa;    // a signal action when CHILD process is finished


    sa.sa_handler = SIG_IGN; // ignore signals - no specific action when SIG_CHILD received
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    // When SIGCHLD received, no action required
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        fprintf(stderr, "sigaction() failed\n");
        exit(1);
    }
    // =========================================================

    Socket socket(this->port);
    socket.createSocket();

    struct sockaddr_in6 server;
    struct sockaddr_in client;

    server.sin6_family = AF_INET6;
    server.sin6_addr = in6addr_any;
    server.sin6_port = htons(this->port); 

    socket.bindSocket(server);

    if (listen(socket.fd, QUEUE) != 0)
    {
        fprintf(stderr, "listen() failed\n");
        exit(1);
    }

    printf("* Waiting for incoming connections on port %d (%d)...\n", this->port,server.sin6_port);

    len = sizeof(client);

    signal(SIGINT, sigintHandler);

    while (1)
    {
        // accept a new connection
        if ((newsocket = accept(socket.fd, (struct sockaddr *)&client, (socklen_t*)&len)) == -1)
        {
            fprintf(stderr, "accept() failed\n");
            exit(1);
        }
        // parent process
        if ((pid = fork()) > 0)
        {
            printf("parent: closing newsock and continue to listen to new incoming connections\n");
            close(newsocket);
        }
        // child process that will handle an incoming request
        else if (pid == 0)
        {
            p = (long) getpid();
            printf("* Closing parent's socket fd, my PID=%ld\n",p);
            close(socket.fd);

            // Set socket to unblocking mode
            int flags = fcntl(newsocket, F_GETFL, 0);
            fcntl(newsocket, F_SETFL, flags | O_NONBLOCK);
            this->newsocket = newsocket;

            LDAP ldap(newsocket, this->file);
            ldap.LDAPrun();

            printf("Communication with client finished.\nClosing socket.\n");
            close(newsocket);                      
            exit(0);   
        }
        else
        {
            fprintf(stderr, "fork() failed\n");
            exit(1);
        }
    }
    // close the server 
    printf("*Closing the original socket\n");
    close(socket.fd);                               // close an original server socket
    return 0;
}

/**
 * @brief Method for printing usage of server
 * 
 */
void Server::printUsage()
{
    printf("NAME:\n");
    printf("    isa_ldapserver - LDAP server that retrieves queries from LDAP clients and search for answers in local text database\n");
    printf("\n");
    printf("USAGE:\n");
    printf("    ./isa_ldapserver [options]");
    printf("\n");
    printf("OPTIONS:\n");
    printf("    --help  -h\n");
    printf("    --port  -p  <port>  - default port is set 389\n");
    printf("    --file  -f  <file>\n");

    exit(0);
}

/**
 * @brief Method for handling SIGINT signal
 * 
 * @param signal 
 */
void Server::sigintHandler(int signal)
{
    printf("\n* SIGINT received\n");
    exit(signal);
}

/**
 * @brief Method for checking existence of file
 * 
 * @param file input file
 */
void Server::checkExistenceOfFile(std::string file)
{
    std::ifstream inputFile;
    inputFile.open(file);
    if (!inputFile.is_open())
    {
        fprintf(stderr, "File %s not found!\n", file.c_str());
        exit(1);
    }
    inputFile.close();
}
