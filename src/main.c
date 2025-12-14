#include <stdio.h>
#include <stdlib.h>         // exit(0)
#include <unistd.h>         // fork

#include <X11/Xlib.h>       // xopen/xclose
#include <X11/Xutil.h>      // WM_DELETE_WINDOW
#include <X11/Xatom.h>      // WM_DELETE_WINDOW
#include <X11/keysym.h>     // XK_q

typedef struct
{
    Window window;     // Client windows
    int x, y, w, h; // x, y, width, height
} Client;

Display     *display;
Window      root;
int         screen;
Client      clients[100];
int         client_count = 0;
Atom        wm_delete_window;
Atom        wm_protocols;

void init_x();
void setup_wm();
void manage_window(Window win);
void tile_windows();
void handle_event(XEvent *ev);
    void handle_keypress(XKeyEvent *ev);
void main_loop(void);

int main()
{
    init_x();       // connect to x and steup display
    setup_wm();     // listen to event, grab keys, set atom

    printf("SpringWM started on display %s\n", DisplayString(display));
    printf("Super+Enter:\tOpen kitty\n");
    printf("Super+q:\tclose focused window\n");
    printf("Super+c:\tclose window manager\n");

    main_loop();

    XCloseDisplay(display);
    return 0;
}

void init_x()
{
    if(!(display    = XOpenDisplay(NULL))) exit(1);
    screen          = DefaultScreen(display);
    root            = RootWindow(display, screen);
}

void setup_wm()
{
    // select event we want to listen 
    XSelectInput(display, root, 
        SubstructureRedirectMask |  // manage child window
        SubstructureNotifyMask   |  // notify window create destroy
        KeyPressMask             |
        ButtonPressMask          |
        PropertyChangeMask);

    // grab some keys 
    XGrabKey(display, XKeysymToKeycode(display, XK_Return),
            Mod4Mask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(display, XKeysymToKeycode(display, XK_q),
            Mod4Mask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(display, XKeysymToKeycode(display, XK_c),
            Mod4Mask, root, True, GrabModeAsync, GrabModeAsync);
    
    // set up window manager name 
    XStoreName(display, root, "SpringWM");

    // set support for WM_DELETE_WINDOW protocol
    wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
    wm_protocols     = XInternAtom(display, "WM_PROTOCOLS", False);
}

void manage_window(Window win)
{
    // store client 
    clients[client_count].window = win;
    client_count++;

    // select events for this window
    XSelectInput(display, win, 
                StructureNotifyMask |
                PropertyChangeMask  |
                KeyPressMask);
    
    // set border 
    XSetWindowBorder(display, win, 0xFF0000);
    XSetWindowBorderWidth(display, win, 0);

    // send WM_DELETE_WINDOW protocol to window 
    XSetWMProtocols(display, win, &wm_delete_window, 1);

    // Focus the window
    XSetInputFocus(display, win, RevertToParent, CurrentTime);

    // tile windows
    tile_windows();
}

void tile_windows()
{
   if (client_count == 0) return; 

    int gap      = 5;       // gap btw window
    int num_cols = (client_count == 1) ? 1 : 2;
    int num_rows = (client_count + num_cols - 1) / num_cols;   

    // calculate available space - gaps
    int total_width  = DisplayWidth(display, screen);
    int total_height = DisplayHeight(display, screen);

    // for gaps btw windows
   int width    = (total_width  - (num_cols + 1) * gap) / num_cols;
   int height   = (total_height - (num_cols + 1) * gap) / num_rows;


   for (int i = 0; i < client_count; i++)
   {
        int col = i % num_cols;
        int row = i / num_cols;

        // added gap position
        clients[i].x = gap + col * (width  + gap);
        clients[i].y = gap + row * (height + gap);
        clients[i].w = width;
        clients[i].h = height;

        XMoveResizeWindow(display, clients[i].window,
            clients[i].x,     clients[i].y,
            clients[i].w, clients[i].h);
            
   }

   XFlush(display);
}

void handle_event(XEvent *ev)
{
    switch(ev->type)
    {
        case MapRequest:
            printf("Window %lu wants to map\n", (unsigned long)ev->xmaprequest.window);
            XMapWindow(display, ev->xmaprequest.window);
            manage_window(ev->xmaprequest.window);
            break;

        case ConfigureRequest:
        {
            // handle window resize/move request from clients
            XConfigureRequestEvent *creq = &ev->xconfigurerequest;
            XWindowChanges changes;
            changes.x               = creq->x;
            changes.y               = creq->y;
            changes.width           = creq->width;
            changes.height          = creq->height;
            changes.border_width    = 0;            // we set borders ourselves
            changes.sibling         = creq->above;
            changes.stack_mode      = creq->detail;
            XConfigureWindow(display, creq->window, creq->value_mask, &changes);
            break;
        }

        case KeyPress:
            handle_keypress(&ev->xkey);
            break;

        case DestroyNotify:
            // remove window form our list 
            for (int i = 0; i < client_count; i++)
            {
                if(clients[i].window == ev->xdestroywindow.window)
                {
                    // shift remaning windows
                    for (int j = i; j < client_count - 1; j++)
                    {
                        clients[j] = clients[j + 1];
                    }
                    client_count--;
                    tile_windows();
                    break;
                }
            }
            break;
        
        case UnmapNotify:
            // window was unmap (minimized or hidden)
            printf("Window %lu unmapped\n", (unsigned long)ev->xunmap.window);
            break;

        case ClientMessage:
            // Handles WM_DELETE_WINDOW protocol
            if(ev->xclient.message_type == wm_protocols &&
                (Atom)ev->xclient.data.l[0] == wm_delete_window)
                {
                    XDestroyWindow(display, ev->xclient.window);
                }
            break;
    }
}

// handle keyboard short cut
void handle_keypress(XKeyEvent *ev)
{
    KeySym keysym = XKeycodeToKeysym(display, ev->keycode, 0);    

    if((ev->state & Mod4Mask) && keysym == XK_Return)
    {
        // Super + Enter: Open terminal 
        pid_t pid = fork();
        if(pid == 0)
        {
            execlp("kitty", "kitty", NULL);
            // if kitty failed
            execlp("xterm", "xterm", NULL);
            exit(1);
        }
        else if(pid < 0)
        {
            printf("NO Xterm found\n");    
        }
    }
    else if ((ev->state & Mod4Mask) && keysym == XK_q)
    {
        // Super + q closed focused window
        Window focused;
        int revert;
        XGetInputFocus(display, &focused, &revert);
        
        if (focused != None && focused != root)         // cant close the root window
        {
            // send WM_DELETE_WINDOW message if supported
            XEvent event                = {0};
            event.type                  = ClientMessage;
            event.xclient.window        = focused;
            event.xclient.message_type  = wm_protocols;
            event.xclient.format        = 32;
            event.xclient.data.l[0]     = wm_delete_window;
            event.xclient.data.l[1]     = CurrentTime;

            XSendEvent(display, focused, False, NoEventMask, &event);
        }
    }
    else if ((ev->state & Mod4Mask) && keysym == XK_c)
    {
        // Super + c: exit wm
        printf("Exiting SpringWm\n");

        // close all client window first 
        for (int i = 0; i < client_count; i++)
        {
            XDestroyWindow(display, clients[i].window);
        }

        // Give X server time to process destroy request 
        XSync(display, False);
        usleep(100000); // 100ms delay

        XCloseDisplay(display);
        printf("SpringWM exited successfully\n");
        exit(0);
    }
}

void main_loop(void)
{
    XEvent ev;

    while (1)
    {
        XNextEvent(display, &ev);
        handle_event(&ev);
    }
}
