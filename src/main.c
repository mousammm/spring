#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>

typedef struct
{
    Window win;     // Client windows
    int x, y, w, h; // x, y, width, height
} Client;

Display     *display;
Window      root;
int         screen;
Client      clients[100];
int         client_count = 0;

void init_x();

int main()
{
    init_x();       // connect to x and steup display
    
    

    return EXIT_SUCCESS;
}

void init_x()
{
    if(!(display    = XOpenDisplay(NULL))) exit(1);
    screen          = DefaultScreen(display);
    root            = RootWindow(display, screen);
}

