#include<stdio.h>
#include<curses.h>
int main()
{
    int i;
    initscr();
    clear();
    for(i=0;i<LINES;i++)
    {
        move(i,i+i);
        if(i%2==1)
            standout();
        addstr("Hello,world");
        if(i%2==1)
            standend();
        refresh();
        sleep(1);
        move(i,i+i);
        addstr("           ");
    }
    endwin();
    return 0;
}
