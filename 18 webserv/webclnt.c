#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<stdlib.h>
#include<stdlib.h>
#include<strings.h>
#define oops(msg) {perror(msg); exit(1);}
int connect_to_server(char *host,int portnum)
{
    struct sockaddr_in servadd;
    struct hostent *hp;
    int sock_id;
    sock_id=socket(AF_INET,SOCK_STREAM,0);
    if(sock_id==-1)
        oops("socket");
    bzero(&servadd,sizeof(servadd));
    hp=gethostbyname(host);
    if(hp==NULL)
        oops(host);
    bcopy(hp->h_addr,(struct sockaddr*)&servadd.sin_addr,hp->h_length);
    servadd.sin_port=htons(portnum);
    servadd.sin_family=AF_INET;
    if(connect(sock_id,(struct sockaddr*)&servadd,sizeof(servadd))!=0)
        oops("connect");
    return sock_id;
}
int main(int argc,char *argv[])
{
    struct sockaddr_in servadd;
    struct hostent *hp;
    int sock_id,sock_fd;
    char message[BUFSIZ];
    int messlen,n_read;
    sock_id=connect_to_server(argv[1],atoi(argv[2]));
    read(0, message, BUFSIZ);
    if(write(sock_id,message,BUFSIZ)==-1)
        oops("write");
    while((n_read=read(sock_id,message,BUFSIZ))>0)
        if(write(1,message,n_read)==-1)
            oops("write");
    close(sock_id);
    return 0;
}

