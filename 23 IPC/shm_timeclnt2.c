#include<stdio.h>
#include<sys/shm.h>
#include <sys/ipc.h>
#include<time.h>
#include<sys/sem.h>
#include<sys/types.h>
#include<string.h>
#include<stdlib.h>
#include<signal.h>
#define	TIME_MEM_KEY	993			/* like a filename      */
#define	TIME_SEM_KEY	9903
#define SEG_SIZE ((size_t)100)
#define oops(m,x) {perror(m); exit(x);}
union semun {int val; struct semid_ds *buf; ushort *array;};

int main()
{
    int seg_id;
    char *mem_ptr,*ctime();
    time_t now;
    int semset_id;
    
    seg_id = shmget( TIME_MEM_KEY, SEG_SIZE, 0777 );
    if(seg_id==-1)
        oops("shmget",1);
    mem_ptr=shmat(seg_id,NULL,0);
    if(mem_ptr==(void*)-1)
        oops("shmattach",2);
    semset_id=semget(TIME_SEM_KEY,2,0);
    wait_and_lock(semset_id);
    printf("The time,direct from memory:..%s\n",mem_ptr);
    release_lock(semset_id);
    shmdt(mem_ptr);
    return 0;
}
void wait_and_lock(int semset_id)
{
    union semun sem_info;
    struct sembuf actions[2];
    actions[0].sem_num=1;
    actions[0].sem_flg=SEM_UNDO;
    actions[0].sem_op=0;

    actions[1].sem_num=0;
    actions[1].sem_flg=SEM_UNDO;
    actions[1].sem_op=+1;
    if(semop(semset_id,actions,2)==-1)
        oops("semop:locking",10);
}
void release_lock(int semset_id)
{
    union semun sem_info;
    struct sembuf actions[1];
    actions[0].sem_num=0;
    actions[0].sem_flg=SEM_UNDO;
    actions[0].sem_op=-1;
    if(semop(semset_id,actions,1)==-1)
        oops("semop:unlocking",10);
}

