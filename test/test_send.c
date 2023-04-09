#include <stdio.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#define BUFFER_SIZE 128

int main(int argc, char *argv[]){
	int sockfd;
	char buf[BUFFER_SIZE] = "Hello Server";
	struct sockaddr_in servaddr;	
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){		//创建socket
		perror("socket");
		exit(-1);
	}
	//设置sockaddr_in结构体中相关参数
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(6666);
	servaddr.sin_addr.s_addr = inet_addr("0.0.0.0");
	
	if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1){
		perror("connect");		//调用connect()向服务器端建立TCP连接
		exit(-1);
	}
	close(sockfd);
	int n = send(sockfd, buf, sizeof(buf), 0);	
    printf("%d %d %s\n",n,errno,strerror(errno));
    n = send(13, buf, sizeof(buf), 0);	
    printf("%d %d %s\n",n,errno,strerror(errno));
    n = send(0, buf, sizeof(buf), 0);	
    printf("%d %d %s\n",n,errno,strerror(errno));
	if(recv(sockfd, buf, sizeof(buf), 0) == -1){
		perror("recv");
		exit(-1);
	}
	printf("Recv from server: %s\n",buf);
	close(sockfd);
	exit(0);
}