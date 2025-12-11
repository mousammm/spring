#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

Display *display;               // connection to x server
Window root;                    // root window

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
	display = XOpenDisplay(NULL);
    if (!display) return 1;

    int screen = DefaultScreen(display);
    root = RootWindow(display, screen);
    printf("Root window ID: %lu\n", root);

    XSelectInput(display, root, 
        SubstructureRedirectMask |
        SubstructureNotifyMask |
        KeyPressMask);

    XGrabKey(display, XKeysymToKeycode(display, XK_Return), 
        Mod4Mask, root, True, GrabModeAsync, GrabModeAsync);

    XGrabKey(display, XKeysymToKeycode(display, XK_q),
        Mod4Mask, root, True, GrabModeAsync, GrabModeAsync);


    printf("\n\n==========================\n");
    printf("SPRING started!\n");
    printf("SUPER+Q: quit!\n");
    printf("SUPER+Enter: Open kitty!\n");
    printf("==========================\n\n");

    XEvent event;
    int running = true;

    while(running)
    {
        XNextEvent(display, &event);

        switch(event.type)
        {
            case KeyPress:

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
