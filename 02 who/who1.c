#include<stdio.h>
#include<utmp.h>
#include<fcntl.h>
#include<unistd.h>
#define SHOWHOST
void show_info(struct utmp *utbufp)
{
    printf("%-8.8s",utbufp->ut_name);
    printf(" ");
    printf("%-8.8s",utbufp->ut_line);
    printf(" ");
    printf("%10ld",utbufp->ut_time);
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
