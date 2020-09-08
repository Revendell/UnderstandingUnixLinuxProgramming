#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<string.h>
#include<sys/wait.h>
#define DFL_PROMPT ">"
#define YES 1
#define NO 0
#define is_delim(x) ((x)==' '||(x)=='\t')
enum states {NEUTRAL,WANT_THEN,THEN_BLOCK};
enum results {SUCCESS,FAIL};
static int if_state=NEUTRAL;
static int if_result=SUCCESS;
static int last_state=0;
void setup()
{
    signal(SIGINT,SIG_IGN);
    signal(SIGQUIT,SIG_IGN);
}
void fatal(char *s1,char *s2,int n)
{
    fprintf(stderr,"Error:%s,%s\n",s1,s2);
    exit(n);
}
int execute(char *argv[])
{
    int pid;
    int child_info=-1;
    if(argv[0]==NULL)
        return 0;
    if((pid=fork())==-1)
        perror("fork");
    else if(pid==0)
    {
        signal(SIGINT,SIG_DFL);
        signal(SIGQUIT,SIG_DFL);
        execvp(argv[0],argv);
        perror("cannot execute command");
        exit(1);
    }
    else
    {
        if(wait(&child_info)==-1)
            perror("wait");
    }
    return child_info;
}
void *emalloc(size_t n)
{
    void *rv;
    if((rv=malloc(n))==NULL)
        fatal("out of memory","",1);
    return rv;
}
void *erealloc(void *p,size_t n)
{
    void *rv;
    if((rv=realloc(p,n))==NULL)
        fatal("realloc() failed","",1);
    return rv;
}
void freelist(char **list)
{
    char **cp=list;
    while(*cp)
        free(*cp++);
    free(list);
}
char *next_cmd(char *prompt,FILE *fp)
{
    char *buf;
    int bufspace=0;
    int pos=0;
    char c;
    printf("%s",prompt);
    while((c=getc(fp))!=EOF)
    {
        //需要空间时申请空间
        if(pos+1>=bufspace)
        {
            if(bufspace==0)  //第一次申请空间
                buf=emalloc(BUFSIZ);
            else  //扩展空间
                buf=erealloc(buf,bufspace+BUFSIZ);
            bufspace+=BUFSIZ;
        }
        if(c=='\n')
            break;
        buf[pos++]=c;
    }
    if(c==EOF&&pos==0)
        return NULL;
    buf[pos]='\0';
    return buf;
}
char *newstr(char *s,int l)
{
    char *rv=emalloc(l+1);
    rv[l]='\0';
    strncpy(rv,s,l);
    return rv;
}
char **splitline(char *line)
{
    char **args;
    int spots=0;
    int bufspace=0;
    int argnum=0;
    char *cp=line;
    char *start;
    int len;
    if(line==NULL)
        return NULL;
    args=emalloc(BUFSIZ);
    bufspace=BUFSIZ;
    spots=BUFSIZ/sizeof(char *);
    while(*cp!='\0')
    {
        while(is_delim(*cp))
            cp++;
        if(*cp=='\0')
            break;
        //make sure the array has room (+1 for NULL)
        if(argnum+1>=spots)
        {
            args=erealloc(args,bufspace+BUFSIZ);
            bufspace+=BUFSIZ;
            spots+=(BUFSIZ/sizeof(char *));
        }
        start=cp;
        len=1;
        while(*++cp!='\0'&&!(is_delim(*cp)))
            len++;
        args[argnum++]=newstr(start,len);
    }
    args[argnum]=NULL;
    return args;
}
int syn_err(char *msg)
{
    if_state=NEUTRAL;
    fprintf(stderr,"syntax error:%s\n",msg);
    return -1;
}
int is_control_command(char *s)
{
    return (strcmp(s,"if")==0||strcmp(s,"then")==0||strcmp(s,"fi")==0);
}
int do_control_command(char **args)
{
    char *cmd=args[0];
    int rv=-1;
    if(strcmp(cmd,"if")==0)
    {
        if(if_state!=NEUTRAL)
            rv=syn_err("if unexpected");
        else
        {
            last_state=process(args+1);
            if_result=(last_state==0?SUCCESS:FAIL);
            if_state=WANT_THEN;
            rv=0;
        }
    }
    else if(strcmp(cmd,"then")==0)
    {
        if(if_state!=WANT_THEN)
            rv=syn_err("then unexpected");
        else
        {
            if_state=THEN_BLOCK;
            rv=0;
        }
    }
    else if(strcmp(cmd,"fi")==0)
    {
        if(if_state!=THEN_BLOCK)
            rv=syn_err("fi unexpected");
        else
        {
            if_state=NEUTRAL;
            rv=0;
        }
    }
    else
        fatal("internal error processing:",cmd,2);
    return rv;
}
int ok_to_execute()
{
    int rv=1;
    if(if_state==WANT_THEN)
    {
        syn_err("then expected");
        rv=0;
    }
    else if(if_state==THEN_BLOCK&&if_result==SUCCESS)
        rv=1;
    else if(if_state==THEN_BLOCK&&if_result==FAIL)
        rv=0;
    return rv;
}
int process(char **args)
{
    int rv=0;
    if(args[0]==NULL)
        rv=0;
    else if(is_control_command(args[0]))
        rv=do_control_command(args);
    else if(ok_to_execute())
        rv=execute(args);
    return rv;
}
int main()
{
    char *cmdline,*prompt,**arglist;
    int result;
    prompt=DFL_PROMPT;
    setup();
    while((cmdline=next_cmd(prompt,stdin))!=NULL)
    {
        if((arglist=splitline(cmdline))!=NULL)
        {
            result=process(arglist);
            freelist(arglist);
        }
        free(cmdline);
    }
    return 0;
}
