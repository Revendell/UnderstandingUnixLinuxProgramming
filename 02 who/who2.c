#include<stdio.h>
#include<utmp.h>
#include<fcntl.h>
#include<unistd.h>
#include<time.h>
#define SHOWHOST
void showtime(long timeval)
{
    char *cp;
    cp=ctime(&timeval);
    printf("%12.12s",cp+4);

}
void show_info(struct utmp *utbufp)
{
    if(utbufp->ut_type!=USER_PROCESS)
        return;
    printf("%-8.8s",utbufp->ut_name);
    printf(" ");
    printf("%-8.8s",utbufp->ut_line);
    printf(" ");
    showtime(utbufp->ut_time);
    printf(" ");
#ifdef SHOWHOST
    printf("(%s)",utbufp->ut_host);
#endif
    printf("\n");
}
int main()
{
    struct utmp current_record;
    int fd;
    int recordlen=sizeof(current_record);
    if((fd=open(UTMP_FILE,O_RDONLY))==-1)
    {
        perror(UTMP_FILE);
	exit(1);
    }
    while(read(fd,&current_record,recordlen)==recordlen)
        show_info(&current_record);
    close(fd);
    return 0;
}
