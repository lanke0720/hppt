#include "window.h"
chtype v = '*';
//初始化
window::window()
{
    initscr();

}

//清除
void window::clrNumsLines(WINDOW* win,int begin,int nums)
{
    while(nums-->0)
    {
    wmove(win,begin++/*y*/,0/*x*/);
    wclrtoeol(win);
    }
}

void window::getStringFromWin(WINDOW* win,std::string &outString)
{
    char buf[1024];
    //O(1)置空
    buf[0] = 0;
    wgetnstr(win,buf,sizeof(buf));
    outString = buf;

}

void window::putStringToWin(WINDOW* win,int start_y,int start_x,\
                            std::string &msg)
{
    mvwaddstr(win,start_y,start_x,msg.c_str());
}

void window::wrefreshMyWindow(WINDOW* win)
{
    wrefresh(win);
}

void window::createHeader()
{
    int h = LINES/5;
    int w = COLS;
    int y = 0;
    int x = 0;
    header = newwin(h,w,y,x);
    box(header,v,v);
}

void window::createoutput()
{
    int h = (LINES*3)/5;
    int w = COLS*3/4;
    int y = LINES/5;
    int x = 0;
    output = newwin(h,w,y,x);
    box(output,v,v);
}

void window::createfriendList()
{
    int h = (LINES*3)/5;
    int w = COLS/4;
    int y = LINES/5;
    int x = COLS*3/4;
    friend_list = newwin(h,w,y,x);
    box(friend_list,v,v);
}

void window::createinput()
{
    
    int h = LINES/5;
    int w = COLS;
    int y = LINES*4/5;
    int x = 0;
    input = newwin(h,w,y,x);
    box(input,v,v);
}

window::~window()
{
    delwin(header);
    delwin(output);
    delwin(friend_list);
    delwin(input);
    endwin();
}

#if 0
int main()
{
    window win;
    //绘制窗口
    win.createHeader();
    win.wrefreshMyWindow(win.header);
   win.createoutput();
    win.wrefreshMyWindow(win.output);
   win.createfriendList();
    win.wrefreshMyWindow(win.friend_list);
   win.createinput();
    win.wrefreshMyWindow(win.input);
    std::string msg = "Welcome To Chat System!";
    int h=0;
    int w=0;

    int step = 0;
    while(1){
    win.createHeader();
    //获得窗口的最大的坐标
    getmaxyx(win.header,h,w); 
    //把数据放到窗口
    win.putStringToWin(win.header,h/2, step++,msg);
    //刷新窗口
    win.wrefreshMyWindow(win.header);
    win.clrNumsLines(win.header,h/2,1);
    step%=w;
    usleep(300000);
    std::string str;
    win.getStringFromWin(win.input,str);

    getmaxyx(win.output,h,w); 
    win.putStringToWin(win.output,1,1,str);
    win.wrefreshMyWindow(win.output);
    }
    return 0;
}

#endif
