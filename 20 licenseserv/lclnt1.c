#include<stdio.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdbool.h>
#include<netinet/in.h>
#define MSGLEN 128
#define SERVER_PORTNUM 2020
#define HOSTLEN 512
#define oops(p) {perror(p);exit(1);}
static int pid=-1;
static int sd=-1;
static struct sockaddr serv_addr;
static socklen_t serv_alen;
static char ticket_buf[128];
static bool have_ticket=false;
int make_dgram_client_socket(char *host,int portnum,struct sockaddr_in *servadd)
{
    struct hostent *hp;
    int sock_id;
    sock_id=socket(PF_INET,SOCK_DGRAM,0);
    if(sock_id==-1)
        oops("socket");
    bzero((void *)servadd,sizeof(struct sockaddr_in));
    hp=gethostbyname(host);
    if(hp==NULL)
        oops(host);
    bcopy((void*)hp->h_addr,(void*)&servadd->sin_addr,hp->h_length);
    servadd->sin_port=htons(portnum);
    servadd->sin_family=AF_INET;
    return sock_id;
}
void setup()
{
    char hostname[BUFSIZ];
    pid=getpid();
    gethostname(hostname,HOSTLEN);
    sd=make_dgram_client_socket(hostname,SERVER_PORTNUM,&serv_addr);
    if(sd==-1)
        oops("cannot make socket");
    serv_alen=sizeof(serv_addr);
}
void shut_down()
{
    close(sd);
}
char *do_transaction(char *msg)
{
    static char buf[MSGLEN];
    int ret=sendto(sd,msg,strlen(msg),0,&serv_addr,serv_alen);
    if(ret==-1)
        oops("sendto");
    ret=recvfrom(sd,buf,MSGLEN,0,NULL,NULL);
    buf[ret]='\0';
    if(ret==-1)
        oops("recvfrom");
    return buf;
}
void narrate(char *msg1,char *msg2)
{
    fprintf(stderr,"CLIENT[%d]:%s %s\n",pid,msg1,msg2);
}
syserr(char *msg1)
{
    char buf[MSGLEN];
    sprintf(buf,"CLIENT[%d]:%s\n",pid,msg1);
    perror(buf);
}
int get_ticket()
{
    char *response;
    char buf[MSGLEN];
    if(have_ticket)
        return 0;
    sprintf(buf,"HELO %d",pid);
    //client将HELO pid发给server，server再传回消息
    if((response=do_transaction(buf))==NULL)
        return -1;
    //如果传回来TICK %d.%d，将%d. %d保存至ticket_buf，获得ticket槽位
    if(strncmp(response,"TICK",4)==0)
    {
        strcpy(ticket_buf,response+5);
        have_ticket=true;
        narrate("got a ticket",ticket_buf);
        return 0;
    }
    //如果传回来FAIL，获取ticket失败
    if(strncmp(response,"FAIL",4)==0)
        narrate("could not get ticket",response);
    else
        narrate("unknown message:",response);
    return -1;
}
void do_regular_work()
{
    printf("SuperSleep version 1.0 Running-Licensed Software\n");
    sleep(10);
}
int release_ticket()
{
    char buf[MSGLEN];
    char *response;
    if(!have_ticket)
        return 0;
    sprintf(buf,"GBYE %s",ticket_buf);
    //client睡眠10秒后，发送GBYE %d. %d给server，server再传回消息
    if((response=do_transaction(buf))==NULL)
        return -1;
    //如果传回来THNX，server收回了ticket
    if(strncmp(response,"THNX",4)==0)
    {
        have_ticket=false;
        narrate("release ticket OK","");
        return 0;
    }
    //如果传回来FAIL，release ticket失败
    else if(strncmp(response,"FAIL",4)==0)
        narrate("release failed",response);
    else
        narrate("unknown message:",response);
    return -1;
}
int main()
{
    setup();
    if(get_ticket()!=0)
        exit(0);
    do_regular_work();
    release_ticket();
    shut_down();
    return 0;
}

