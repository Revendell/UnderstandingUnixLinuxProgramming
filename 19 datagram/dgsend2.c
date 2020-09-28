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
int make_dgram_client_socket(char *host,int portnum,struct sockaddr_in *servadd)
{
    struct hostent *hp;
    int sock_id;
    sock_id=socket(PF_INET,SOCK_DGRAM,0);
    if(sock_id==-1)
        oops("socket",1);
    bzero((void *)servadd,sizeof(struct sockaddr_in));
    hp=gethostbyname(host);
    if(hp==NULL)
        oops(host,2);
    bcopy((void*)hp->h_addr,(void*)&servadd->sin_addr,hp->h_length);
    servadd->sin_port=htons(portnum);
    servadd->sin_family=AF_INET;
    return sock_id;
}
int main(int argc,char *argv[])
{
    int sock;
    char *msg;
    char buf[BUFSIZ];
    size_t msglen;
    struct sockaddr_in saddr;
    if(argc!=4)
    {
        fprintf(stderr,"usage:dgsend host portnum message\n");
        exit(1);
    }
    if((sock=make_dgram_client_socket(argv[1],atoi(argv[2]),&saddr))==-1)
        oops("cannot make socket",2);
    if(sendto(sock,argv[3],sizeof(argv[3]),0,(const struct sockaddr *)&saddr,sizeof(saddr))==-1)
        oops("sendto",3);
    msglen = recvfrom(sock,buf,BUFSIZ,0,NULL,NULL);
	buf[msglen] = '\0';
	printf("dgsend: got a message: %s\n", buf);
	return 0;
}

