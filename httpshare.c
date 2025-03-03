#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #pragma comment(lib, "Ws2_32.lib")
    #include <Winsock2.h>
#else
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <unistd.h>
#endif

#define BUF_SIZE 1024*1024

char buf[BUF_SIZE];

long getFileSize(char* file) {
    FILE* fp = fopen(file, "rb");
    if (fp == NULL) {
        return 0;
    }
    fseek(fp, 0L, SEEK_END);
    long size = ftell(fp);
    fclose(fp);
    return size;
}

char* getFileName(char *path)
{
    int len = strlen(path);
    int flag=0;
     
    for(int i=len-1; i>0; i--)
    {
        if(path[i]=='\\' || path[i] == '//' || path[i]=='/' )
        {
            flag=1;
            path = path+i+1;
            break;
        }
    }
    return path;
}

int createServerSocket(int port) {
    int server_fd;
    struct sockaddr_in address;

#ifdef _WIN32
    WSADATA wsaData;

    if (WSAStartup(0x202, &wsaData) != 0) {
        printf("ERROR: WSAStartup failure.\n");
        exit(EXIT_FAILURE);
    }
#endif

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    return server_fd;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Shoud use with <port> <file>\r\n");
        return 1;
    }

    short port = strtol(argv[1], NULL, 10);
    char* filePath = argv[2];

    long fileSize = getFileSize(filePath);
    if (fileSize == 0) {
        printf("Couldn't get file\n");
        return 1;
    }

    int serverSocket = createServerSocket(port);
    printf("Waiting for connection\r\n");
    int clientSocket = accept(serverSocket, NULL, NULL);

    if (clientSocket < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    int requestBytes = recv(clientSocket, buf, BUF_SIZE, 0);
    if (requestBytes <= 0) {
        printf("read request error\r\n");
        goto closeSockets;
    }

    sprintf(buf, "HTTP/1.1 200 OK\n"
       "Content-Disposition: attachment; filename=\"%s\"\n"
       "Content-Type: application/octet-stream\n"
       "Keep-Alive: timeout=2, max=1\n"
       "Connection: Keep-Alive\n"
       "Content-Length: %ld\n\n", getFileName(filePath), fileSize);
    send(clientSocket, buf, strlen(buf), 0);

    FILE* file = fopen(filePath, "rb");
    int fd = fileno(file);
    int i = 0;

    while (i < fileSize) {
        int actualRead = read(fd, buf, BUF_SIZE);
        i += actualRead;
        send(clientSocket, buf, actualRead, 0);
    }
    fclose(file);
    printf("File sent\r\n");


closeSockets:;
    shutdown(clientSocket, SHUT_RDWR);
    int optReuse = 1;
    setsockopt(serverSocket,SOL_SOCKET,SO_REUSEADDR, &optReuse, sizeof(int));
    close(clientSocket);
    close(serverSocket);

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}