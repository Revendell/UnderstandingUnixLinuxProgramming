#include<curses.h>
#include<sys/time.h>
#include<signal.h>
#define BLANK ' '
#define DFL_SYMBOL 'o'
#define TOP_ROW 5
#define BOT_ROW 20
#define LEFT_EDGE 10
#define RIGHT_EDGE 70
#define X_INIT 10
#define Y_INIT 10
#define TICKS_PER_SEC 50  //1/50second
#define X_TTM 5 //移动间隔信号数
#define Y_TTM 8
struct ppball{
    int y_pos,x_pos,
        y_ttm,x_ttm,
        y_ttg,x_ttg,
        y_dir,x_dir;
    char symbol;
};
struct ppball the_ball;
int bounce_or_lose(struct ppball *bp)
{
    int return_val=0;
    if(bp->y_pos==TOP_ROW)
    {
        bp->y_dir=1;
        return_val=1;
    }
    else if(bp->y_pos==BOT_ROW)
    {
        bp->y_dir=-1;
        return_val=1;
    }
    if(bp->x_pos==LEFT_EDGE)
    {
        bp->x_dir=1;
        return_val=1;
    }
    else if(bp->x_pos==RIGHT_EDGE)
    {
        bp->x_dir=-1;
        return_val=1;
    }
    return return_val;
}
void ball_move(int signum)
{
    int y_cur,x_cur,moved;
    signal(SIGALRM,SIG_IGN);
    y_cur=the_ball.y_pos;
    x_cur=the_ball.x_pos;
    moved=0;
    if(the_ball.y_ttm>0&&the_ball.y_ttg--==1)
    {
        the_ball.y_pos+=the_ball.y_dir;
        the_ball.y_ttg=the_ball.y_ttm;
        moved=1;
    }
    if(the_ball.x_ttm>0&&the_ball.x_ttg--==1)
    {   
        the_ball.x_pos+=the_ball.x_dir;
        the_ball.x_ttg=the_ball.x_ttm;
        moved=1;
    }
    if(moved)
    {
        mvaddch(y_cur,x_cur,BLANK);
        mvaddch(the_ball.y_pos,the_ball.x_pos,the_ball.symbol);
        bounce_or_lose(&the_ball);
        move(LINES-1,COLS-1);
        refresh();
    }
    signal(SIGALRM,ball_move);
}
int set_ticker(int n_msecs)
{
    struct itimerval new_timeset;
    long n_sec,n_usecs;
    n_sec=n_msecs/1000;
    n_usecs=(n_msecs%1000)*1000L;
    new_timeset.it_interval.tv_sec=n_sec;
    new_timeset.it_interval.tv_usec=n_usecs;
    new_timeset.it_value.tv_sec=n_sec;
    new_timeset.it_value.tv_usec=n_usecs;
    return setitimer(ITIMER_REAL,&new_timeset,NULL);
}
void set_up()
{
    the_ball.y_pos=Y_INIT;
    the_ball.x_pos=X_INIT;
    the_ball.y_ttg=the_ball.y_ttm=Y_TTM;
    the_ball.x_ttg=the_ball.x_ttm=X_TTM;
    the_ball.y_dir=the_ball.x_dir=1;
    the_ball.symbol=DFL_SYMBOL;
    initscr();
    crmode();
    noecho();
    signal(SIGINT,SIG_IGN);
    mvaddch(the_ball.y_pos,the_ball.x_pos,the_ball.symbol);
    refresh();
    signal(SIGALRM,ball_move);
    set_ticker(1000/TICKS_PER_SEC);  //millisecs per ticker
}
void wrap_up()
{
    set_ticker(0);
    endwin();
}
int main()
{
    int c;
    set_up();
    while((c=getchar())!='q')
    {
        if(c=='d') the_ball.x_ttm--;
        else if(c=='a') the_ball.x_ttm++;
        else if(c=='w') the_ball.y_ttm--;
        else if(c=='s') the_ball.y_ttm++;
    }
    wrap_up();
    return 0;
}
