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
#define HOSTLEN 256
#define BACKLOG 1
#define oops(msg) {perror(msg); exit(1);}
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
void header(FILE *fp,char *content_type)
{
    fprintf(fp,"HTTP/1.0 200 OK\r\n");
    if(content_type)
        fprintf(fp,"Content-type:%s\r\n",content_type);
}
void cannot_do(int fd)
{
    FILE *fp=fdopen(fd,"w");
    fprintf(fp,"HTTP/1.0 501 Not Implemented\r\n");
    fprintf(fp,"Content-type:text/plain\r\n");
    fprintf(fp,"That command is not yet implement\r\n");
    fclose(fp);
}
bool not_exist(char *f)
{
    struct stat info;
    //stat把文件fname的信息复制到指针bufp所指的结构
    return (stat(f,&info)==-1);
}
void do_404(char *item,int fd)
{
    FILE *fp=fdopen(fd,"w");
    fprintf(fp,"HTTP/1.0 404 Not Found\r\n");
    fprintf(fp,"Content-type:text/plain\r\n");
    fprintf(fp,"The item you requested:%s is not found\r\n",item);
    fclose(fp);
}
bool isadir(char *f)
{
    struct stat info;
    //stat把文件fname的信息复制到指针bufp所指的结构
    return (stat(f,&info)!=-1 && S_ISDIR(info.st_mode));
}
void do_ls(char *dir,int fd)
{
    FILE *fp=fdopen(fd,"w");
    header(fp,"text/plain");
    fprintf(fp,"\r\n");
    fflush(fp);
    //标准输入输出全部定向到缓存区，然后通过socket传给客户端
    dup2(fd,1);
    dup2(fd,2);
    close(fd);
    execlp("ls","ls","-l",dir,NULL);
    perror(dir);
    exit(1);
}
//得到文件后缀名
char *file_type(char *f)
{
    char *cp;
    if((cp=strrchr(f,'.'))!=NULL)
        return cp+1;
    return "";
}
bool ends_in_cgi(char *f)
{
    return (strcmp(file_type(f),"cgi")==0);
}
void do_exec(char *prog,int fd)
{
    FILE *fp=fdopen(fd,"w");
    header(fp,"text/plain");
    fprintf(fp,"\r\n");
    fflush(fp);
    //标准输入输出全部定向到缓存区，然后通过socket传给客户端
    dup2(fd,1);
    dup2(fd,2);
    close(fd);
    execl(prog,prog,NULL);
    perror(prog);
    exit(1);
}
void do_cat(char *f,int fd)
{
    char *extension=file_type(f);
    char *content="text/plain";
    FILE *fpsock,*fpfile;
    char c;
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
        header(fpsock,content);
        fprintf(fpsock,"\r\n");
        while((c=getc(fpfile))!=EOF)
            putc(c,fpsock);
        fclose(fpfile);
        fclose(fpsock);
    }
    exit(0);
}
void process_rq(char *rq,int fd)
{
    char cmd[BUFSIZ],arg[BUFSIZ];
    if(fork()!=0)
        return;
    if(sscanf(rq,"%s %s",cmd,arg)!=2)
        return;
    if(strcmp(cmd,"GET")!=0)  //必须通过GET实现请求
        cannot_do(fd);
    else if(not_exist(arg))  //arg必须是已存在的文件或目录
        do_404(arg,fd);
    else if(isadir(arg))  //如果arg是目录执行ls命令
        do_ls(arg,fd);
    else if(ends_in_cgi(arg))  //如果arg是.cgi文件执行该文件
        do_exec(arg,fd);
    else                       //如果是其他文件执行cat
        do_cat(arg,fd);
}
int main(int argc,char *argv[])
{
    int sock,fd;
    FILE *fpin;
    char request[BUFSIZ];
    if(argc==1)
        oops("usage:webserv portnum\n");
    sock=make_server_socket(atoi(argv[1]));
    if(sock==-1)
        oops("sock");
    while(1)
    {
        fd=accept(sock,NULL,NULL);
        fpin=fdopen(fd,"r");
        fgets(request,BUFSIZ,fpin);
        printf("got a call:request=%s",request);
        int len = strlen(request);
        request[len-1]=0;  //去掉\n
        process_rq(request,fd);
        fclose(fpin);
    }
    return 0;
}
