#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<string.h>
#include<stdlib.h>
#define oops(m) {perror(m);exit(1);}
#define SOCKNAME "/tmp/logfilesock"
int main(int argc,char *argv[])
{
    int sock;
    struct sockaddr_un addr;
    socklen_t addrlen;
    char *msg=argv[1];
    char sockname[]=SOCKNAME;
    if(argc!=2)
        oops("usage:logfileclnt massege\n");
    sock=socket(PF_UNIX,SOCK_DGRAM,0);
    addr.sun_family=AF_UNIX;
    strcpy(addr.sun_path,sockname);
    addrlen=strlen(sockname)+sizeof(addr.sun_family);
    if(sock==-1)
        oops("socket");
    if(sendto(sock,msg,strlen(msg),0,(const struct sockaddr *)&addr,addrlen)==-1)
        oops("sendto");
    close(sock);
    return 0;
}

