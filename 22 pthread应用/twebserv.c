#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<netdb.h>
#include<time.h>
#include<stdbool.h>
#include<sys/stat.h>
#include<string.h>
#include<pthread.h>
#include<dirent.h>
#define HOSTLEN 256
#define BACKLOG 1
#define oops(msg) {perror(msg); exit(1);}
time_t server_started;
int server_bytes_sent;
int server_requests;
int make_server_socket_q(int portnum,int backlog)
{
    struct sockaddr_in saddr;
    struct hostent *hp;
    char hostname[HOSTLEN];
    int sock_id;
    sock_id=socket(PF_INET,SOCK_STREAM,0);
    if(sock_id==-1)
        oops("socket");
    bzero((void*)&saddr,sizeof(saddr));
    gethostname(hostname,HOSTLEN);
    hp=gethostbyname(hostname);
    bcopy((void*)hp->h_addr,(void*)&saddr.sin_addr,hp->h_length);
    saddr.sin_port=htons(portnum);
    saddr.sin_family=AF_INET;
    if(bind(sock_id,(struct sockaddr*)&saddr,sizeof(saddr))!=0)
        oops("bind");
    if(listen(sock_id,backlog)!=0)
        oops("listen");
    return sock_id;
}
int make_server_socket(int portnum)
{
    return make_server_socket_q(portnum,BACKLOG);
}
int http_reply(int fd,FILE **fpp,int code,char *msg,char *type,char *content)
{
    FILE *fp=fdopen(fd,"w");
    int bytes=0;
    if(fp!=NULL)
    {
        bytes=fprintf(fp,"HTTP/1.0 %d %s\r\n",code,msg);
        bytes+=fprintf(fp,"Content-type:%s\r\n",type);
        if(content)
            bytes+=fprintf(fp,"%s\r\n",content);
    }
    fflush(fp);
    if(fpp)
        *fpp=fp;
    else
        close(fp);
    return bytes;
}
void cannot_do(int fd)
{
    http_reply(fd,NULL,501,"Not Implemented","text/plain","That command is not yet implement");
}
bool not_exist(char *f)
{
    struct stat info;
    //stat把文件fname的信息复制到指针bufp所指的结构
    return (stat(f,&info)==-1);
}
void do_404(char *item,int fd)
{
    http_reply(fd,NULL,404,"Not Found","text/plain","The item you seek is not here");
}
bool isadir(char *f)
{
    struct stat info;
    //stat把文件fname的信息复制到指针bufp所指的结构
    return (stat(f,&info)!=-1 && S_ISDIR(info.st_mode));
}
void do_ls(char *dir,int fd)
{
    DIR *dirptr;
    struct dirent *direntp;
    int bytes=0;
    FILE *fp;
    bytes=http_reply(fd,&fp,200,"OK","text/plain",NULL);
    bytes+=fprintf(fp,"Listing of Directory %s\n",dir);
    if((dirptr=opendir(dir))!=NULL)
    {
        while(direntp=readdir(dirptr))
        {
            bytes+=fprintf(fp,"%s\n",direntp->d_name);
        }
        closedir(dirptr);
    }
    fclose(fp);
    server_bytes_sent+=bytes;
}
//得到文件后缀名
char *file_type(char *f)
{
    char *cp;
    if((cp=strrchr(f,'.'))!=NULL)
        return cp+1;
    return "";
}
void do_cat(char *f,int fd)
{
    char *extension=file_type(f);
    char *content="text/plain";
    FILE *fpsock,*fpfile;
    char c;
    int bytes=0;
    if(strcmp(extension,"html")==0)
        content="text/html";
    else if(strcmp(extension,"gif")==0)
        content="text/gif";
    else if(strcmp(extension,"jpg")==0)
        content="text/jpeg";
    else if(strcmp(extension,"jpeg")==0)
        content="text/jpeg";
    fpsock=fdopen(fd,"w");
    fpfile=fopen(f,"r");
    if(fpsock!=NULL && fpfile!=NULL)
    {
        bytes=http_reply(fd,&fpsock,200,"OK",content,NULL);
        while((c=getc(fpfile))!=EOF)
            putc(c,fpsock);
        fclose(fpfile);
        fclose(fpsock);
    }
    server_bytes_sent+=bytes;
}
int built_in(char *arg,int fd)
{
    FILE *fp;
    if(strcmp(arg,"status")!=0)
        return 0;
    http_reply(fd,&fp,200,"OK","text/plain",NULL);
    fprintf(fp,"Server started:%s",ctime(&server_started));
    fprintf(fp,"Total requests:%d\n",server_requests);
    fprintf(fp,"Bytes sent out:%d\n",server_bytes_sent);
    fclose(fp);
    return 1;
}
void process_rq(char *rq,int fd)
{
    char cmd[BUFSIZ],arg[BUFSIZ];
    if(sscanf(rq,"%s %s",cmd,arg)!=2)
        return;
    if(strcmp(cmd,"GET")!=0)  //必须通过GET实现请求
        cannot_do(fd);
    else if(built_in(arg,fd))  //如果arg是status返回服务器状态
        ;
    else if(not_exist(arg))  //arg必须是已存在的文件或目录
        do_404(arg,fd);
    else if(isadir(arg))  //如果arg是目录执行ls命令
        do_ls(arg,fd);
    else                   //如果是其他文件执行cat
        do_cat(arg,fd);
}
void setup(pthread_attr_t *attrp)
{
    pthread_attr_init(attrp);
    pthread_attr_setdetachstate(attrp,PTHREAD_CREATE_DETACHED);
    time(&server_started);
    server_requests=0;
    server_bytes_sent=0;
}
void handle_call(int *fdptr)
{
    FILE *fpin;
    char request[BUFSIZ];
    int fd=*fdptr;
    free(fdptr);
    fpin=fdopen(fd,"r");
    fgets(request,BUFSIZ,fpin);
    printf("got a call on %d:request=%s",fd,request);
    process_rq(request,fd);
    fclose(fpin);
}
int main(int argc,char *argv[])
{
    int sock,fd;
    int *fdptr;
    pthread_t worker;
    pthread_attr_t attr;
    if(argc==1)
        oops("usage:twebserv portnum\n");
    sock=make_server_socket(atoi(argv[1]));
    if(sock==-1)
        oops("sock");
    setup(&attr);
    while(1)
    {
        fd=accept(sock,NULL,NULL);
        server_requests++;
        fdptr=malloc(sizeof(int));
        *fdptr=fd;
        pthread_create(&worker,&attr,handle_call,fdptr);
    }
    return 0;
}
