#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>
#define MSGLEN 512
#define oops(m) {perror(m);exit(1);}
#define SOCKNAME "/tmp/logfilesock"
int main()
{
    int sock;
    struct sockaddr_un addr;
    socklen_t addrlen;
    char msg[MSGLEN];
    int l;
    char sockname[]=SOCKNAME;
    time_t now;
    int msgnum=0;
    char *timestr;
    addr.sun_family=AF_UNIX;
    strcpy(addr.sun_path,sockname);
    addrlen=strlen(sockname)+sizeof(addr.sun_family);
    sock=socket(PF_UNIX,SOCK_DGRAM,0);
    if(sock==-1)
        oops("socket");
    if(bind(sock,(struct sockaddr *)&addr,addrlen)==-1)
        oops("bind");
    while(1)
    {
        l=read(sock,msg,MSGLEN);
        msg[l]='\0';
        time(&now);
        timestr=ctime(&now);
        timestr[strlen(timestr)-1]='\0';
        printf("[%d]%s %s\n",msgnum++,timestr,msg);
        fflush(stdout);
    }
    close(sock);
    return 0;
}
