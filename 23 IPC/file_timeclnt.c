#include<stdio.h>
#include<sys/file.h>
#include <sys/ipc.h>
#include<time.h>
#include<fcntl.h>
#include<string.h>
#include<stdlib.h>
#define oops(m,x) {perror(m); exit(x);}
#define BUFLEN 10
void lock_operation(int fd,int op)
{
    struct flock lock;
    lock.l_whence=SEEK_SET;
    lock.l_start=lock.l_len=0;
    lock.l_pid=getpid();
    lock.l_type=op;
    if(fcntl(fd,F_SETLKW,&lock)==-1)
        oops("lock operation",6);
}
int main(int argc,char *argv[])
{
    int fd,nread;
    char buf[BUFSIZ];
    if(argc!=2)
    {
        fprintf(stderr,"usage:%s filename",argv[0]);
        exit(1);
    }
    if((fd=open(argv[1],O_RDONLY))==-1)
        oops(argv[1],2);
    lock_operation(fd,F_RDLCK);
    while((nread=read(fd,buf,BUFSIZ))>0)
        write(1,buf,nread);
    lock_operation(fd,F_UNLCK);
    close(fd);
    return 0;
}

