#include<stdio.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/errno.h>
#include<signal.h>
#include<sys/socket.h>
#include<stdbool.h>
#include<netinet/in.h>
#define MSGLEN 128
#define SERVER_PORTNUM 2020
#define HOSTLEN 512
#define MAXUSERS 3
#define TICKET_AVAIL 0
#define RECLAIM_INTERVAL 30
#define oops(p) {perror(p);exit(1);}
int ticket_array[MAXUSERS];
int sd=-1;
int num_tickets_out=0;
int make_dgram_server_socket(int portnum)
{
    struct sockaddr_in saddr;
    char hostname[HOSTLEN];
    struct hostent *hp;
    int sock_id;
    sock_id=socket(PF_INET,SOCK_DGRAM,0);
    if(sock_id==-1)
        oops("socket");
    bzero((void*)&saddr,sizeof(saddr));
    gethostname(hostname,HOSTLEN);
    hp=gethostbyname(hostname);
    bcopy((void*)hp->h_addr,(void*)&saddr.sin_addr,hp->h_length);
    saddr.sin_port=htons(portnum);
    saddr.sin_family=AF_INET;
    if(bind(sock_id,(struct sockaddr *)&saddr,sizeof(saddr))!=0)
        oops("bind");
    return sock_id;
}

void free_all_tickets()
{
    int i;
    for(i=0;i<MAXUSERS;i++)
        ticket_array[i]=TICKET_AVAIL;
}
int setup()
{
    sd=make_dgram_server_socket(SERVER_PORTNUM);
    if(sd==-1)
        oops("make socket");
    free_all_tickets();
    return sd;
}
void shut_down()
{
    close(sd);
}
void narrate(char *msg1,char *msg2,struct sockaddr_in *clientp)
{
    fprintf(stderr,"\t\tSERVER:%s %s",msg1,msg2);
    if(clientp)
        fprintf(stderr,"(%s:%d)",inet_ntoa(clientp->sin_addr),ntohs(clientp->sin_port));
    putc('\n',stderr);
}
//处理客户机崩溃，收回ticket
void ticket_reclaim()
{
    int i;
    char tick[BUFSIZ];
    for(i=0;i<MAXUSERS;i++)
    {
        if((ticket_array[i]!=TICKET_AVAIL)&&(kill(ticket_array[i],0)==-1)&&(errno==ESRCH))
        {
            sprintf(tick,"%d.%d",ticket_array[i],i);
            narrate("freeing",tick,NULL);
            ticket_array[i]=TICKET_AVAIL;
            num_tickets_out--;
        }
    }
    alarm(RECLAIM_INTERVAL);
}
char *do_hello(char *msg_p)
{
    int x;
    static char replybuf[MSGLEN];
    if(num_tickets_out>=MAXUSERS)
        return "FAIL,no tickets available";
    for(x=0;x<MAXUSERS&&ticket_array[x]!=TICKET_AVAIL;x++);
    if(x==MAXUSERS)
    {
        narrate("database corrupt","",NULL);
        return "FAIL,database corrupt";
    }
    ticket_array[x]=atoi(msg_p+5);
    sprintf(replybuf,"TICK %d.%d",ticket_array[x],x);
    num_tickets_out++;
    return replybuf;
}
char *do_goodbye(char *msg_p)
{
    int pid,slot;
    if((sscanf((msg_p+5),"%d.%d",&pid,&slot)!=2)||(ticket_array[slot]!=pid))
    {
        narrate("bogus ticket",msg_p+5,NULL);
        return "FAIL,invalid ticket";
    }
    ticket_array[slot]=TICKET_AVAIL;
    num_tickets_out--;
    return "THNX see you!";
}
char *do_validate(char *msg)
{
    int pid,slot;
    if((sscanf((msg+5),"%d.%d",&pid,&slot)==2) && (ticket_array[slot]==pid))
    {
        return "GOOD Valid ticket";
    }
    narrate("Bogus ticket",msg+5,NULL);
    return "FAIL invalid ticket";
}
void handle_request(char *req,struct sockaddr_in *client,socklen_t addlen)
{
    char *response;
    int ret;
    //client发送HELO pid过来，server占位ticket_array同时num_tickets_out++并且发送回TICK %d.%d
    if(strncmp(req,"HELO",4)==0)
        response=do_hello(req);
    //client发送GBYE %d.%d过来，server释放ticket_array同时num_tickets_out--并且发送回THNX
    else if(strncmp(req,"GBYE",4)==0)
        response=do_goodbye(req);
    //client发送VALD %d.%d过来，server验证是否存在并且发送回GOOD或者FAIL
    else if(strncmp(req,"VALD",4)==0)
        response=do_validate(req);
    else
        response="FAIL,invalid request";
    narrate("SAID:",response,client);
    ret=sendto(sd,response,strlen(response),0,client,addlen);
    if(ret==-1)
        oops("SERVER sendto failed");
}
int main()
{
    struct sockaddr_in client_addr;
    socklen_t addrlen;
    char buf[MSGLEN];
    int sock,ret;
    unsigned time_left;
    sock=setup();
    signal(SIGALRM,ticket_reclaim);
    alarm(RECLAIM_INTERVAL);
    while(1)
    {
        addrlen=sizeof(client_addr);
        ret=recvfrom(sock,buf,MSGLEN,0,&client_addr,&addrlen);
        if(ret!=-1)
        {
            buf[ret]='\0';
            narrate("GOT:",buf,&client_addr);
            time_left=alarm(0);
            handle_request(buf,&client_addr,addrlen);
            alarm(time_left);
        }
        else if(errno!=EINTR)
            oops("recvfrom");
    }
    return 0;
}

