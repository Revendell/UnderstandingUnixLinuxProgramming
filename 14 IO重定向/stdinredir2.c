#include<stdio.h>
#include<fcntl.h>
int main()
{
    int fd;
    int newfd;
    char line[100];
    fgets(line,100,stdin);
    printf("%s",line);
    fgets(line,100,stdin);
    printf("%s",line);
    fgets(line,100,stdin);
    printf("%s",line);
    fd=open("/etc/passwd",O_RDONLY);
    close(0);
    newfd=dup(fd);
    if(newfd!=0)
    {
        fprintf(stderr,"Could not open data as fd 0\n");
        exit(1);
    }
    close(fd);
    fgets(line,100,stdin);
    printf("%s",line);
    fgets(line,100,stdin);
    printf("%s",line);
    fgets(line,100,stdin);
    printf("%s",line);
    return 0;
}
