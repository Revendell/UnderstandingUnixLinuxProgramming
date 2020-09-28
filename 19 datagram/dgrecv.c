#include<stdio.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#define oops(m,x) {perror(m);exit(x);}
#define HOSTLEN 256
int make_dgram_server_socket(int portnum)
{
    struct sockaddr_in saddr;
    char hostname[HOSTLEN];
    struct hostent *hp;
    int sock_id;
    sock_id=socket(PF_INET,SOCK_DGRAM,0);
    if(sock_id==-1)
        oops("socket",1);
    bzero((void*)&saddr,sizeof(saddr));
    gethostname(hostname,HOSTLEN);
    hp=gethostbyname(hostname);
    bcopy((void*)hp->h_addr,(void*)&saddr.sin_addr,hp->h_length);
    saddr.sin_port=htons(portnum);
    saddr.sin_family=AF_INET;
    if(bind(sock_id,(struct sockaddr *)&saddr,sizeof(saddr))!=0)
        oops("bind",2);
    return sock_id;
}
void say_who_called(struct sockaddr_in *addrp)
{
    char host[BUFSIZ];
    int port;
    strncpy(host,inet_ntoa(addrp->sin_addr),BUFSIZ);
    port=ntohs(addrp->sin_port);
    printf("from:%s:%d\n",host,port);
}
int main(int argc,char *argv[])
{
    int port;
    int sock;
    char buf[BUFSIZ];
    size_t msglen;
    struct sockaddr_in saddr;
    socklen_t saddrlen=sizeof(saddr);
    if(argc!=2 || (port=atoi(argv[1]))<=0)
    {
        fprintf(stderr,"usage:dgrecv portnum\n");
        exit(1);
    }
    if((sock=make_dgram_server_socket(port))==-1)
        oops("cannot make sock",3);
    while((msglen=recvfrom(sock,buf,BUFSIZ,0,(struct sockaddr *)&saddr,&saddrlen))>0)
    {
        buf[msglen]='\0';
        printf("dgrecv:got a message:%s\n",buf);
        say_who_called(&saddr);
    }
    return 0;
}
