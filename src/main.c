#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct Client Client;

struct Client
{
    Window client_win;
    struct Client *next;
};

Client *clients = NULL;

void add_client(Window win);
void spawn_terminal();
void title_windows(Display *display, int screen);

int main()
{
    Display *display;
    Window root;
    XWindowAttributes win_attr;
    XButtonEvent start;         // save pointer state at the start of move/resize
    XEvent event;
    Window selected_win = None;  // track which window move/resiz

	if(!(display = XOpenDisplay(0x0))) return 1;

    int screen = DefaultScreen(display);
    root = RootWindow(display, screen);

    XSelectInput(display, root, 
        SubstructureRedirectMask |
        SubstructureNotifyMask |
        KeyPressMask);

    XGrabKey(display, XKeysymToKeycode(display, XK_Return),         // mod+enter
        Mod4Mask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(display, XKeysymToKeycode(display, XK_q),              // mod+q
        Mod4Mask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabButton(display, 1, Mod4Mask, root, True, ButtonPressMask,  // left mouse
        GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(display, 3, Mod4Mask, root, True, ButtonPressMask,  // right mouse
        GrabModeAsync, GrabModeAsync, None, None);

    int running = true;

    while(running)
    {
        XNextEvent(display, &event);

        switch(event.type)
        {
            case KeyPress:
                if(event.xkey.subwindow != None)
                    XRaiseWindow(display, event.xkey.subwindow);

                KeySym keysys = XKeysymToKeycode(display, event.xkey.keycode);
                printf("Key press: %lu\n", keysys);

                if (event.xkey.keycode == XKeysymToKeycode(display, XK_Return)) 
                {
                    printf("Spaning terminal\n");
                    spawn_terminal();
                }
                else if (event.xkey.keycode == XKeysymToKeycode(display, XK_q)) 
                {
                    printf("Exitng SPUER_WM...\n");
                    XCloseDisplay(display);
                    running = false;
                }

            break;

            case MapRequest:
                printf("CLIENT added to LIST...\n");
                XMapWindow(display, event.xmaprequest.window);
                add_client(event.xmaprequest.window);
                title_windows(display, screen);
            break;

            case DestroyNotify:
                printf("WNIDOW %lu DESTROYED...\n", event.xdestroywindow.window);
                // todo remove from the list
            break;

            case ButtonPress:
                if(event.xkey.subwindow != None)
                {
                    selected_win = event.xbutton.subwindow;
                    XGrabPointer(display, selected_win, True,
                        PointerMotionMask | ButtonReleaseMask, 
                        GrabModeAsync,GrabModeAsync, None, None, CurrentTime);

                    XGetWindowAttributes(display, selected_win, &win_attr);
                    start = event.xbutton;
                    printf("Button press on window %lu, button %d\n", 
                           selected_win, start.button);
                }

            break;
            
            case MotionNotify:
                if(selected_win != None)
                {
                    int xdiff, ydiff;

                    // Discard excess motion events
                    while(XCheckTypedEvent(display, MotionNotify, &event));

                    xdiff = event.xbutton.x_root - start.x_root;
                    ydiff = event.xbutton.y_root - start.y_root;

                    printf("Motion: xdiff=%d, ydiff=%d, button=%d\n", 
                           xdiff, ydiff, start.button);
                    if(start.button == 1)   // left btn move
                    {
                        XMoveWindow(display, selected_win,
                            win_attr.x + xdiff,
                            win_attr.y + ydiff);

                    }
                    else if (start.button == 3) // right btn resize
                    {
                        XResizeWindow(display, selected_win,
                            MAX(1, win_attr.width  + xdiff ),
                            MAX(1, win_attr.height + ydiff ));
                    }
               }

            break;

            case ButtonRelease:
                 XUngrabPointer(display, CurrentTime);
                 selected_win = None;
            break;
        } 

    }

    XCloseDisplay(display);
    return 0;
}

void add_client(Window win)
{
    Client *c = malloc(sizeof(Client));
    c->client_win = win;
    c->next       = clients;
    clients       = c;

    printf("\nCHILD WINDOW: %lu (total: ", win);

    int count = 0;
    for (Client *tmp = clients; tmp; tmp = tmp->next) count++;
    printf("%d)\n", count);
}

void spawn_terminal()
{
    if (fork() == 0) 
    {
        execlp("kitty", "kitty", NULL);
        exit(0);
    }
}

void title_windows(Display *display, int screen)
{
    int num_clients = 0;
    Client *c;

    // count clients
    for (c = clients; c; c = c->next) num_clients++;

    if (num_clients == 0) return;

    int screen_width = DisplayWidth(display, screen);
    int screen_height = DisplayHeight(display, screen);

    int i = 0;
    for (c = clients; c; c = c->next)
    {
        int width = screen_width / num_clients;
        int x = i * width;

        XMoveResizeWindow(display, c->client_win, x, 0, width, screen_height);
        i++;
    }
    XFlush(display);
}
