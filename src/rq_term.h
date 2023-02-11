#pragma once
#include <conio.h>
#include <windows.h>
#include "rq_common.h"

#define CSI "\x1b["

// These are just the ones I am using right now;
// this should be expanded 
typedef enum {
    KEY_ENTER = 13,
    KEY_UP = 72,
    KEY_LEFT = 75,
    KEY_RIGTH = 77,
    KEY_DOWN = 80,
    KEY_Q = 113
} KeyCode;

KeyCode get_key_code() {
    uint kc = _getch();
    if (!kc) { // If kc == 0 we that means we need to check the other
        kc = _getch();
    }
    return (KeyCode) kc;
}

void set_virtual_mode() {
    #ifndef DEBUGGING
    // Set output mode to handle virtual terminal sequences
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        panic("Couldn't go into virtual mode", RQ_ERROR_VIRTUAL_MODE);
    }

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode))
    {
        panic("Couldn't go into virtual mode", RQ_ERROR_VIRTUAL_MODE);
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode))
    {
        panic("Couldn't go into virtual mode", RQ_ERROR_VIRTUAL_MODE);
    }
    printf(CSI "?25l");
    #endif
}

void unset_virtual_mode() {
    printf(CSI "?15h");
}

void clear_screen() {
    printf(CSI "1;1H");
    printf(CSI "2J");
}

char *ask_for_string() {    // I stole this code directly from stack overflow
    int c;                  //as getchar() returns `int`
    char *string = malloc(sizeof(char));
    if (string == NULL) {
        panic("Failed to allocate enough memory", RQ_ERROR_NOT_ENOUGH_MEMORY);
    }

    string[0]='\0';

    for(int i=0; (c=getchar())!='\n' && c != EOF ; i++)
    {
        string = realloc(string, (i+2)*sizeof(char)); //reallocating memory
        if (string == NULL) {
            panic("Failed to reallocate memory", RQ_ERROR_NOT_ENOUGH_MEMORY);
        }
        string[i] = (char) c;   //type casting `int` to `char`
        string[i+1] = '\0';     //inserting null character at the end
    }
    return string;
}

void draw_selection(char **menus, uint cursor_pos, uint length) {
    for (uint i = 0; i < length; i++)
    {
        if (i == cursor_pos)
            printf(">");
        printf("\t%s\n", menus[i]);
    }
    printf("\nQ/LEFT_ARROW quits | ENTER/RIGTH_ARROW selects\n");
}

uint select_from_menu(char *title, char **menus, uint length) {
    KeyCode ch;
    uint cursor_pos = 0;

    set_virtual_mode();

    clear_screen();
    printf("%s", title);
    draw_selection(menus, cursor_pos, length);
    
    while (1)
    {
        if (_kbhit()) {
            ch = get_key_code();
            switch (ch)
            {
                case KEY_ENTER:
                case KEY_RIGTH:
                    return cursor_pos;

                case KEY_UP:
                    if (cursor_pos == 0) {
                        cursor_pos = length -1;
                    } else {
                        cursor_pos = (cursor_pos - 1) % length;
                    }
                    break;

                case KEY_DOWN:
                    cursor_pos = (cursor_pos + 1) % length;
                    break;
                
                case KEY_LEFT:
                case KEY_Q:
                    return length; // We use length as the exit condition

                default:
                    break;
            }
            
            clear_screen();
            printf("%s", title);
            draw_selection(menus, cursor_pos, length);
        }
    }

    unset_virtual_mode();
}
