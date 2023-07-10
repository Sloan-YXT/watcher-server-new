#include <stdio.h>
#include <string.h>
int main(void)
{
    int a=1,b=2,c=3;
    (a=b)=c;
    printf("%d %d %d\n",a,b,c);
    a=(c=b);
    printf("%d %d %d\n",a,b,c);
}