#include<stdio.h>
#include<sys/shm.h>
#include <sys/ipc.h>
#include<time.h>
#include<string.h>
#include<stdlib.h>
#define TIME_MEM_KEY ((key_t)1234)
#define SEG_SIZE ((size_t)100)
#define oops(m,x) {perror(m); exit(x);}
int main()
{
    int seg_id;
    char *mem_ptr,*ctime();
    long now;
    int n;
    seg_id=shmget(TIME_MEM_KEY,SEG_SIZE,0777);
    if(seg_id==-1)
        oops("shmget",1);
    mem_ptr=shmat(seg_id,NULL,0);
    if(mem_ptr==(void*)-1)
        oops("shmattach",2);
    printf("The time,direct from memory:..%s",mem_ptr);
    shmdt(mem_ptr);
    return 0;
}
