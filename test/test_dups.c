#include <stdio.h>
#include <string.h> 
int main(void)
{
    char box[1024]="";
    FILE *file = fopen("testdup","w");
    fprintf(file,"test:%s;1234%d","(%s)1234",1234);
    fclose(file);
    puts(box);
}