#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#define CHILD_MSG "I want a cookie\n"
#define PAR_MSG "testing...\n"
#define oops(m,x) {perror(m);exit(x);}
int main()
{
    int len,pipefd[2];
    char buf[BUFSIZ];
    int read_len;
    if(pipe(pipefd)==-1)
        oops("Could not make pipe",1);
    printf("Got a pipe.It is file descriptors:{%d,%d}\n",pipefd[0],pipefd[1]);
    switch(fork())
    {
        case -1:
            oops("cannot fork",2);
        case 0:
            len=strlen(CHILD_MSG);
            while(1)
            {
                if(write(pipefd[1],CHILD_MSG,len)!=len)
                    oops("wirte",3);
                sleep(5);
            }
        default:
            len=strlen(PAR_MSG);
            while(1)
            {
                if(write(pipefd[1],PAR_MSG,len)!=len)
                    oops("write",4);
                sleep(1);
                read_len=read(pipefd[0],buf,BUFSIZ);
                if(read_len<=0)
                    break;
                write(1,buf,read_len);
            }
    }
    return 0;
}
