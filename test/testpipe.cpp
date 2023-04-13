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
    if (inet_aton("47.108.170.207", &serverAddr.sin_addr) == 0)
    {
        perror("address transferring error");
        exit(1);
    }
    fd = socket(AF_INET, SOCK_STREAM, 0);

    if (connect(fd, (sockaddr *)&serverAddr, sizeof(serverAddr)))
    {
        perror("connect error");
        exit(1);
    }
    long long a = 4;
    char msg[1024];
    int n = send(fd,&a,8,0);
    if(n<0)
    {
        perror("send error");
        exit(1);
    }
    sleep(1);
    for(int i=0; i<10; i++)
    {
        n = send(fd,&a,4,0);
        if(n<0)
        {
            perror("send error");
            exit(1);
        }
        sleep(1);
        cout<<"send turn "+i<<endl;
    }
    n = close(fd);
    if(n<0)
    {
        perror("close error");
        exit(1);
    }
    while (1)
        ;
}