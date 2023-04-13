#include <fcntl.h>
#include <iostream>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
pthread_t a;
int fd;
using namespace std;
int main(void)
{
    struct sockaddr_in serverAddr,clientAddr;
    socklen_t addrlen = sizeof(clientAddr);
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    socklen_t len = sizeof(serverAddr);
    if (inet_aton("0.0.0.0", &serverAddr.sin_addr) == 0)
    {
        perror("address transferring error");
        exit(1);
    }
    fd = socket(AF_INET, SOCK_STREAM, 0);

    if (bind(fd, (sockaddr *)&serverAddr, sizeof(serverAddr)))
    {
        perror("bind error");
        exit(1);
    }
    if(listen(fd, 5) ==-1)
    {
        perror("listen error");
        exit(1);
    
    }
    while(1)
    {
        int connfd = accept(fd,(sockaddr *)&clientAddr,&addrlen);
        int a;
        char msg[1024];
        int n = recv(connfd,&a,4,MSG_WAITALL);
        if(n<0)
        {
            perror("recv error");
            exit(1);
        }
        cout<<a<<endl;
        n = close(connfd);
        if(n<0)
        {
            perror("close error");
            exit(1);
        }
    }
    while (1)
        ;
}