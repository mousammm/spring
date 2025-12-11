# SPRING X Window Manager (Learning Project)

A minimal tiling window manager written in C for educational purposes. Currently in early development with basic X11 connectivity and keyboard input handling.

## Current Status
- Basic X server connection
- Keyboard input handling (basic)
- Tiling layout (buggy)

![SPRING_WM PNG](asset/spring_wm.png)

## Features Implemented
- X11 server connection and event loop
- Basic keyboard shortcuts (SUPER+Enter, SUPER+Q)
- Window creation and management structure
- Initial tiling layout logic

## TODO List

### High Priority
- [ ] Fix linked list implementation for client windows
- [ ] Implement proper error handling for X11 calls
- [ ] Create stable tiling algorithm (master-stack layout)
- [ ] Add window focus tracking
- [ ] Implement window borders and decorations

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

## Keybindings (Current)
- `SUPER + Enter`: Launch terminal
- `SUPET + Q`: Close window manger itself

## Learning Goals
- Understand X11 protocol and window management
- Master low-level C memory management
- Learn event-driven programming patterns
- Practice system-level programming concepts

## Resources
- Xlib Manual
- X Window System Protocol
- Existing WMs: dwm, i3, awesomewm source code

---
*Note: This project is for educational purposes. Use at your own risk on production systems.*
