#include<stdio.h>
#include<pthread.h>
#include<ctype.h>
#include<stdlib.h>
struct arg_set{
    char *fname;
    int count;
};
void count_words(struct arg_set *args)
{
    char *f=args->fname;
    FILE *fp;
    char c,prevc='\0';
    if((fp=fopen(f,"r"))!=NULL)
    {
        while((c=getc(fp))!=EOF)
        {
            //单词与单词之间有非字母数字间隔
            if(!isalnum(c)&&isalnum(prevc))
                args->count++;
            prevc=c;
        }
        fclose(fp);
    }
    else
        perror(f);
}
int main(int argc,char *argv[])
{
    pthread_t t1,t2;
    struct arg_set args1,args2;
    if(argc!=3)
    {
        printf("usage:%s file1 file2",argv[0]);
        exit(1);
    }
    args1.fname=argv[1];
    args1.count=0;
    args2.fname=argv[2];
    args2.count=0;
    pthread_create(&t1,NULL,count_words,&args1);
    pthread_create(&t2,NULL,count_words,&args2);
    pthread_join(t1,NULL);
    pthread_join(t2,NULL);
    printf("%d:total words\n",args1.count+args2.count);
    return 0;
}

