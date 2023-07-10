#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include "unistd.h"
int main(void)
{
    int fd = open("test.txt",O_CREAT|O_WRONLY);
    close(fd);
    printf("%d,%s\n",errno,strerror(errno));
    close(fd);
    printf("%d,%s\n",errno,strerror(errno));
    
}