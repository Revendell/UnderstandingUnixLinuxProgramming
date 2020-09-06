#include<stdio.h>
#include<signal.h>
#include<string.h>
#include<stdlib.h>
#define MAXARGS 20
#define ARGLEN 100
int excute(char *arglist[])
{
    int pid,exitstatus;
    pid=fork();
    switch(pid)
    {
        case -1:
            perror("fork failed");
            exit(1);
        case 0:
            execvp(arglist[0],arglist);
            perror("execvp");
            exit(1);
        default:
            while(wait(&exitstatus)!=pid)
                ;
            printf("child exited with status %d,%d\n",exitstatus>>8,exitstatus&0377);
    }

}
char *makestring(char *buf)
{
    char *cp;
    buf[strlen(buf)-1]='\0';
    cp=malloc(strlen(buf)+1);
    if(cp==NULL)
    {
        fprintf(stderr,"no memory\n");
        exit(1);
    }
    strcpy(cp,buf);
    return cp;
}
int main()
{
    char *arglist[MAXARGS+1];
    int numargs=0;
    char argbuf[ARGLEN];
    while(numargs<MAXARGS)
    {
        printf("Arg[%d]?",numargs);
        if(fgets(argbuf,ARGLEN,stdin)&&*argbuf!='\n')
            arglist[numargs++]=makestring(argbuf);
        else
        {
            if(numargs>0)
            {
                arglist[numargs]=NULL;
                excute(arglist);
                numargs=0;
            }
        }
    }
    return 0;
}
