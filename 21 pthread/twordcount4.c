#include<stdio.h>
#include<pthread.h>
#include<ctype.h>
#include<stdlib.h>
struct arg_set{
    char *fname;
    int count;
};
struct arg_set *mailbox;
pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t flag=PTHREAD_COND_INITIALIZER;
void count_words(struct arg_set *args)
{
    char *f=args->fname;
    FILE *fp;
    char c,prevc='\0';
    if((fp=fopen(f,"r"))!=NULL)
    {
        while((c=getc(fp))!=EOF)
        {
            //�����뵥��֮���з���ĸ���ּ��
            if(!isalnum(c)&&isalnum(prevc))
                args->count++;
            prevc=c;
        }
        fclose(fp);
    }
    else
        perror(f);
    printf("CPUNT:waiting to get lock\n");
    //���뱣����������lock��flag����ֹ�߳�ͬʱ��������
    pthread_mutex_lock(&lock);
    printf("COUNT:have lock,storing data\n");
    //ֻ��mailboxΪ��ʱ��ռ��mailbox����������̵߳ȴ��ź�
    if(mailbox!=NULL)
        pthread_cond_wait(&flag,&lock);
    mailbox=args;
    printf("COUNT:raising flag\n");
    //�����źŻ��ѵȴ����߳�
    pthread_cond_signal(&flag);
    printf("COUNT:unlocking box\n");
    pthread_mutex_unlock(&lock);
}
int main(int argc,char *argv[])
{
    pthread_t t1,t2;
    struct arg_set args1,args2;
    int reports_in=0;
    int total_words=0;
    if(argc!=3)
    {
        printf("usage:%s file1 file2",argv[0]);
        exit(1);
    }
    //���뱣����
    pthread_mutex_lock(&lock);
    args1.fname=argv[1];
    args1.count=0;
    args2.fname=argv[2];
    args2.count=0;
    pthread_create(&t1,NULL,count_words,&args1);
    pthread_create(&t2,NULL,count_words,&args2);
    while(reports_in<2)
    {
        printf("MAIN:waiting for flag to go up\n");
        //�����̵߳ȴ��ź�
        pthread_cond_wait(&flag,&lock);
        printf("MAIN:flag was raised,I have the lcok\n");
        printf("%d:%s\n",mailbox->count,mailbox->fname);
        total_words+=mailbox->count;
        if(mailbox==&args1)
            pthread_join(t1,NULL);
        if(mailbox==&args2)
            pthread_join(t2,NULL);
        mailbox=NULL;
        pthread_cond_signal(&flag);
        reports_in++;
    }
    printf("%d:total words\n",args1.count+args2.count);
    return 0;
}

