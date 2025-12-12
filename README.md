# SPRING X Window Manager (Learning Project)

A minimal tiling window manager written in C for educational purposes. Currently in early development with basic X11 connectivity and keyboard input handling.

## Current Status

![SPRING_WM PNG](asset/spring_wm.png)

## TODO List
- [x] X11 server connection and event loop
- [x] Basic keyboard shortcuts (SUPER+Enter, SUPER+Q)
- [x] Initial tiling layout logic
- [x] Mouse resize and move 

## Building & Running
```bash
make
make run
```

## Dependencies
- X11 development libraries
- GCC or Clang
- Make
- Xephyr

## Resources
- Xlib Manual
- X Window System Protocol
- Existing WMs: dwm, i3, tinyWM, awesomewm source code

---
*Note: This project is for educational purposes. Use at your own risk on production systems.*
