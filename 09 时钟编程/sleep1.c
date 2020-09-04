#include<stdio.h>
#include<signal.h>
void wakeup()
{
    printf("Alarm received from kernel\n");
}
int main()
{
    printf("about to sleep for 4 seconds\n");
    signal(SIGALRM,wakeup);
    alarm(4);
    pause();
    printf("Morning so soon?\n");
    return 0;
}
