#include<stdio.h>
#include<curses.h>
#include<signal.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/time.h>
#define MESSAGE "hello"
#define BLANK "     "
int row;
int col;
int dir;
int delay;
void move_msg(char *msg)
{
    while(1)
    {
        usleep(delay*1000);
        move(row,col);
        addstr(BLANK);
        col+=dir;
        move(row,col);
        addstr(MESSAGE);
        refresh();
        if(dir==-1&&col<=0)
            dir=1;
        else if(dir==1&&col+strlen(msg)>=COLS)
            dir=-1;
    }
}
int main()
{
    int newdelay;
    char c;
    pthread_t msg_thread;
    initscr();
    crmode();
    noecho();
    clear();
    row=10;
    col=10;
    dir=1;
    delay=200;  //200ms=0.2seconds
    if(pthread_create(&msg_thread,NULL,move_msg,MESSAGE))
    {
        fprintf(stderr,"error creating thread");
        endwin();
        exit(0);
    }
    while(1)
    {
        newdelay=0;
        c=getch();
        if(c=='q') break;
        if(c==' ') dir=-dir;
        if(c=='f'&&delay>2) newdelay=delay/2;
        if(c=='s') newdelay=delay*2;
        if(newdelay>0)
            delay=newdelay;
    }
    pthread_cancel(msg_thread);
    endwin();
    return 0;
}

