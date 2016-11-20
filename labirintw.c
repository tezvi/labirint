/* labirintw.c */
/* Template file for labirint game */
/* D.G. */

#include <curses.h>
#include <menu.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "game.h"

#define MENU_NEW_GAME  0
#define MENU_CREDITS   1
#define MENU_EXIT      2

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

char *menu_choices[] = {
    "  New game  ",
    "  Credits  ",
    "  Exit  "
};

char *smiley[] = {
"    ..::''''::..",
"  .;''        ``;.",
" ::    ::  ::    ::",
"::     ::  ::     ::",
":: .:' ::  :: `:. ::",
"::  :          :  ::",
" :: `:.      .:' ::",
"  `;..``::::''..;'",
"    ``::,,,,::''"
};


static int init_menu();
void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color);
void print_smiley(WINDOW *win, int x, int y);
void show_credits();
void fatal_error(char *msg);

int main (void) {
    int choice=-1;

    initscr();
    if (has_colors () == TRUE) {
        start_color ();
    } else {
        exit (EXIT_FAILURE);
    }

    noecho ();
    keypad (stdscr, TRUE);

    // setup color pairs
    init_pair(1,COLOR_WHITE,COLOR_BLACK);   // labirint
    init_pair(2,COLOR_MAGENTA,COLOR_BLACK); // put, obavijesti
    init_pair(3,COLOR_GREEN,COLOR_BLACK);   // hint ruta
    init_pair(4,COLOR_RED,COLOR_BLACK);     // ghosts
    init_pair(5,COLOR_BLUE,COLOR_BLACK);    // header, footer

    while(1) {
        choice = init_menu();

        if (choice == MENU_EXIT) {
            break;
        } else if (choice == MENU_NEW_GAME) {
            init_game(LINES, COLS);
            clean_game();
        } else if (choice == MENU_CREDITS) {
            show_credits();
            refresh();
        }
    }

    clear();
    refresh();
    endwin();
    printf("Bye!\n");

    return EXIT_SUCCESS;
}


static int init_menu() {
    ITEM **my_items;
    int c, n_choices, i, choice=-1, w, h;
    MENU *my_menu;
    WINDOW *my_menu_win;

    clear();
    wrefresh(stdscr);

    n_choices = ARRAY_SIZE(menu_choices);
    if ((my_items = (ITEM **)calloc(n_choices + 1, sizeof(ITEM *))) == NULL)
        fatal_error("Cannot allocate menu memory");
    for(i = 0; i < n_choices; ++i) {
        my_items[i] = new_item(menu_choices[i], "");
        item_opts_off(my_items[i], O_SHOWDESC);
    }
    my_items[n_choices] = (ITEM *)NULL;

    /* Create menu */
    my_menu = new_menu((ITEM **)my_items);

    w = COLS-8; h = LINES-6;
    my_menu_win = newwin(h, w, 3, 4);
    keypad(my_menu_win, TRUE);

    set_menu_win(my_menu, my_menu_win);
    set_menu_sub(my_menu, derwin(my_menu_win, 3, 30, 7, w-30));

	/* Set menu mark to the string " * " */
    set_menu_mark(my_menu, " -> ");

	/* printaj border */
    box(my_menu_win, 0, 0);
	print_in_middle(my_menu_win, 1, 0, w, "-=! LABIRINT !=-", A_BOLD | COLOR_PAIR(3));
	mvwprintw(my_menu_win, 1, w-5, "v0.3");
	mvwaddch(my_menu_win, 2, 0, ACS_LTEE);
	mvwhline(my_menu_win, 2, 1, ACS_HLINE, w-1);
	mvwaddch(my_menu_win, 2, w-1, ACS_RTEE);

	print_smiley(my_menu_win, 5, 10);
	refresh();

    // boje menija
    set_menu_fore(my_menu, COLOR_PAIR(1) | A_REVERSE);
    set_menu_back(my_menu, COLOR_PAIR(3));
    set_menu_grey(my_menu, COLOR_PAIR(2));

    print_in_middle(my_menu_win, h-2, 1, w, "Press <ENTER> to select an option, Up and Down keys to navigate", COLOR_PAIR(1));
    post_menu(my_menu);
    wrefresh(my_menu_win);

    while((c = wgetch(my_menu_win)) != -1)
    {
        switch(c) {
            case KEY_DOWN:
                menu_driver(my_menu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(my_menu, REQ_UP_ITEM);
                break;
            case 10: /* Enter */
                move(0, 0);
                clrtoeol();
                choice = item_index(current_item(my_menu));
                break;
        }

        if (choice > -1) break;
    }
    unpost_menu(my_menu);
    delwin(my_menu_win);

    if (free_menu(my_menu) != E_OK)
        fatal_error("Cannot free menu memory");
    for(i = 0; i < n_choices; ++i)
        free_item(my_items[i]);
    free(my_items);

    return choice;
}

void show_credits() {
    WINDOW *cwin = newwin(8, 45, LINES/2-5, COLS/2-22);
    wattrset (cwin, A_BOLD | COLOR_PAIR (5));
    box(cwin, 0, 0);
    print_in_middle(cwin, 2, 0, 45, "Based on labirintw game", COLOR_PAIR(1));
    print_in_middle(cwin, 3, 0, 45, "By D.G., rewritten by Vitez", COLOR_PAIR(1));
    wattrset (cwin, ~A_BOLD & COLOR_PAIR (5));
    print_in_middle(cwin, 5, 0, 45, "Press <ENTER> to close", COLOR_PAIR(1));
    //wmove(cwin, 0,0);
    while(wgetch(cwin) != 10);
    delwin(cwin);
}

void fatal_error(char *msg) {
    clean_game();
    clear();
    refresh();
    endwin();
    printf("A fatal error has occurred!\ndesc: %s\n", msg);
    exit(1);
}

void print_smiley(WINDOW *win, int y, int x) {
    int i;
    for(i=0; i<ARRAY_SIZE(smiley); i++) {
        mvwprintw(win, y+i, x, "%s", smiley[i]);
    }
}

void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color)
{	int length, x, y;
	float temp;

	if(win == NULL)
		win = stdscr;
	getyx(win, y, x);
	if(startx != 0)
		x = startx;
	if(starty != 0)
		y = starty;
	if(width == 0)
		width = 80;

	length = strlen(string);
	temp = (width - length)/ 2;
	x = startx + (int)temp;
	wattron(win, color);
	mvwprintw(win, y, x, "%s", string);
	wattroff(win, color);
	refresh();
}
