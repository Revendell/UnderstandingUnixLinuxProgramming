#include<stdio.h>
#include<pthread.h>
#include<ctype.h>
#include<stdlib.h>
int total_words=0;
void count_words(char *f)
{
    FILE *fp;
    char c,prevc='\0';
    if((fp=fopen(f,"r"))!=NULL)
    {
        while((c=getc(fp))!=EOF)
        {
            //单词与单词之间有非字母数字间隔
            if(!isalnum(c)&&isalnum(prevc))
                total_words++;
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
    if(argc!=3)
    {
        printf("usage:%s file1 file2",argv[0]);
        exit(1);
    }
    total_words=0;
    pthread_create(&t1,NULL,count_words,argv[1]);
    pthread_create(&t2,NULL,count_words,argv[2]);
    pthread_join(t1,NULL);
    pthread_join(t2,NULL);
    printf("%d:total words\n",total_words);
    return 0;
}
