#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<stdlib.h>
#include<strings.h>
#include<string.h>
#define oops(msg) {perror(msg); exit(1);}
#define PORTNUM 15000
int main(int argc,char *argv[])
{
    struct sockaddr_in servadd;
    struct hostent *hp;
    int sock_id,sock_fd;
    char buffer[BUFSIZ];
    int n_read;
    if(argc!=3)
        exit(1);
    sock_id=socket(AF_INET,SOCK_STREAM,0);
    if(sock_id==-1)
        oops("socket");
    bzero(&servadd,sizeof(servadd));
    hp=gethostbyname(argv[1]);
    if(hp==NULL)
        oops(argv[1]);
    bcopy(hp->h_addr,(struct sockaddr*)&servadd.sin_addr,hp->h_length);
    servadd.sin_family=AF_INET;
    servadd.sin_port=htons(PORTNUM);
    if(connect(sock_id,(struct sockaddr*)&servadd,sizeof(servadd))!=0)
        oops("connect");
    if(write(sock_id,argv[2],strlen(argv[2]))==-1)
        oops("write");
    if(write(sock_id,"\n",1)==-1)
        oops("write");
    while((n_read=read(sock_id,buffer,BUFSIZ))>0)
    {
        if(write(1,buffer,n_read)==-1)
            oops("write");
    }
    close(sock_id);
    return 0;
}
