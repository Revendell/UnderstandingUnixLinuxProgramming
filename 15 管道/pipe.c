#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#define CHILD_MSG "I want a cookie\n"
#define PAR_MSG "testing...\n"
#define oops(m,x) {perror(m);exit(x);}
int main(int argc,char **argv)
{
    int thepipe[2],newfd,pid;
    if(argc!=3)
    {
        fprintf(stderr,"usage:pipe cmd1 cmd2\n");
        exit(1);
    }
    if(pipe(thepipe)==-1)
        oops("Could not make pipe",1);
    if((pid=fork())==-1)
        oops("Could not fork",2);
    if(pid>0)
    {
        close(thepipe[1]);
        if(dup2(thepipe[0],0)==-1)
            oops("could not redirect stdin",3);
        close(thepipe[0]);
        execlp(argv[2],argv[2],NULL);
        oops(argv[2],4);
    }
    else if(pid==0)
    {
        close(thepipe[0]);
        if(dup2(thepipe[1],1)==-1)
            oops("could not redirect stdout",4);
        close(thepipe[1]);
        execlp(argv[1],argv[1],NULL);
        oops(argv[1],5);
    }
    return 0;
}

