#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <iostream>
#include <curses.h>

class window{
public:
    window();
    void wrefreshMyWindow(WINDOW* win);
    void putStringToWin(WINDOW* win);
    void putStringToWin(WINDOW* win,int start_y,int start_x,std::string &msg);
    void getStringFromWin(WINDOW* win,std::string &outString);
    void clrNumsLines(WINDOW* win,int begin,int nums);
    void createHeader();
    void createoutput();
    void createfriendList();
    void createinput();
    ~window();
public:
    WINDOW* header;
    WINDOW* output;
    WINDOW* friend_list;
    WINDOW* input;
};


#endif
