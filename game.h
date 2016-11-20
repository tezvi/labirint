#include <curses.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define ROWS 50
#define COLM 200

#define SKILL_LENGTH 40

#define QUIT 113  // 'q'
#define LEFT KEY_LEFT   // '<-'
#define RIGHT KEY_RIGHT  // '->'
#define ADVANCE 32 // ' '
#define KEY_HINT 104 // hint key
#define KEY_SKILL 115 // skill toggle key
#define KEY_TRAIL 116 // trail toggle key

#define STRING_ON "on"
#define STRING_OFF "off"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

void init_game(int p_lines, int p_cols);
void print_dialog(char *msg, chtype color, int width, int height);
void clean_game();
void free_ghosts();
void free_labirint();

extern void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color);
extern void fatal_error(char *msg);
