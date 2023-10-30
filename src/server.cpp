#include "../include/server.h"
#include "../include/ldapParser.h"

Server::Server()
{
    this->port = PORT;
    this->file = "";
}

/**
 * Funtion parse arguments from command line to port and file
 * 
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

int Server::run()
{
    // pak asi predelat ========================================
    int newsocket;
    int len, msgSize, i;
    char buffer;
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
    // memset(&server, 0, sizeof(server));
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
            // p = (long) getpid();
            // printf("* Closing parent's socket fd, my PID=%ld\n",p);
            // close(socket.fd);

            // printf("* A new connection accepted from  %s, port %d (%d)\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port), client.sin_port);
            // strcpy(buffer,"200 OK: Echo server2 is listening\n"); // sends confirmation to the client
            // msgSize = strlen(buffer);
            // i = write(newsocket,buffer,msgSize);

            // if (i != msgSize)
            // {
	        //     err(1,"initial write() failed");
            // }

            ldapParser ldapParser;
            ldapParser.LDAPparse(newsocket);
            
            

            // while ((msgSize = read(newsocket, buffer, BUFFER)) > 0)
            // {
            //     printf("received data = \"%.*s\"\n",msgSize-2,buffer); // tear off CR+LF from the receiving buffer
	
            //     for (i = 0; i < msgSize; i++)
            //     {
            //         if (islower(buffer[i]))
            //         {
            //             buffer[i] = toupper(buffer[i]);
            //         }
            //         else if (isupper(buffer[i]))
            //         {
            //             buffer[i] = tolower(buffer[i]);
            //         }
            //     }  
	
	        //     i = write(newsocket,buffer,msgSize);    // send a converted message to the client
	        //     if (i == -1)
            //     {
	        //         err(1,"write() failed.");
            //     }                           // check if data was successfully sent out
	        //     else if (i != msgSize)
            //     {
	        //         err(1,"write(): buffer written partially");
            //     }
            // }
            // printf("* Closing newsock\n");
            close(newsocket);                          // close the new socket
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
 * Funtion prints usage of program
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