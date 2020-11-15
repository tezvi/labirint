#include "game.h"

typedef struct s_ghost {
    int pos;
    int ex_pos;
    int trend;
    int changes;
} sghosts;

static bool **labir = NULL;
static sghosts **ghosts = NULL;
static int xo, yo, x, y;
static int nrupa = 5;
static int nlab, mlab;
static int hint = 0;
static int distance = 0;
static int skill_level = 3;
static int show_trail = 1;
static WINDOW *win;

static float skill_factors[] = {
        0, 0.50f, 0.20f, 0.05f, 0.025f
};


static int get_res(bool **p_rows, int *res, int pos, int row, int width, int height);

static void generate_labirint(int n, int m, int rupa);

static void print_map(int lin, int col, WINDOW *win);

static void print_player(int x, int y, int sx, int sy, WINDOW *win);

static void print_hints(int y, int x, int res[], int cnt, WINDOW *win);

static void clear_hints();

static void print_route(int lin, int col, int pos, int row, WINDOW *win);

static void init_ghosts(int lines, int cols);

static void evolve_ghosts();

static void print_ghosts(WINDOW *);

static float get_skill_factor();

static void update_footer();

/**
* args **p_rows pointer na polje labirint
* args *res pointer na prvi element output polja
* args pos trenutna pozicija igraca
* args width sirina jednog reda
* args height broj redaka u labirintu
* return int broj pronaslih rupa, moze posluziti za kontrolu da li smo zaista izasli iz labirinta
*/

static int get_res(bool **p_rows, int *res, int pos, int row, int width, int height) {
    int k, i, z, cnt = 0;

    if (row > height) return -1;

    for (k = row; k < height; k++) {

        // mozda stojimo na rupi?
        if (p_rows[k][pos] == 1) {
            *(res + cnt++) = 0;
            continue;
        }
        for (i = pos - 1, z = pos + 1; i >= 0 || z < width; i--, z++) {
            // provjeri poziciju lijevo
            if (i >= 0) {
                if (p_rows[k][i] == 1) {
                    *(res + cnt++) = i - pos;
                    pos = i;
                    break;
                }
            }
            // provjeri poziciju desno
            if (z < width) {
                if (p_rows[k][z] == 1) {
                    *(res + cnt++) = z - pos;
                    pos = z; //cnt++;
                    break;
                }
            }
        }
    }

    // vracamo broj pronaslih rupa, moze posluziti za kontrolu
    // da li smo zaista izasli iz labirinta
    return cnt;
}


static void generate_labirint(int n, int m, int rupa) {

    int i, j, r;

    srand((unsigned) time(NULL));

    if (labir != NULL)
        free_labirint();

    if ((labir = (bool **) malloc(n * sizeof(bool *))) == NULL)
        fatal_error("Cannot allocate memory for labirint");

    for (i = 0; i < n; i++) {
        if ((labir[i] = (bool *) calloc(sizeof(bool), m)) == NULL)
            fatal_error("Cannot allocate memory for labirint row");
    }

    // init
    for (i = 0; i < n; i++) {
        for (j = 0; j < rupa;) {
            r = rand() % m;
            if (labir[i][r] == 0) {
                labir[i][r] = 1;
                j++;
            }
        }
    }
}


static void print_map(int lin, int col, WINDOW *win) {
    int i, j, n, m, r;
    chtype zn;
    int a, b;

    if (win) {
        getbegyx(win, yo, xo);
        getmaxyx(win, b, a);
    } else return;

    n = (lin > b - 3 ? b - 3 : lin);
    m = (col > a ? a : col);

    r = 0;
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            zn = (labir[i][j] == 0 ? '=' : ' ');
            mvwaddch(win, 2 + r, j, ' ');
            mvwaddch(win, 3 + r, j, zn);
        }
        r += 2;
    }

    for (i = 0; i < m; i++) mvwaddch(win, r + 2, i, ' ');
    wrefresh(win);
}

static void print_player(int x, int y, int sx, int sy, WINDOW *win) {
    wattrset(win, A_BOLD | COLOR_PAIR (2));

    if (show_trail) {
        if (sy != y) {
            mvwaddch(win, sy, sx, ACS_TTEE);
            mvwaddch(win, y - 1, sx, ACS_VLINE);
        } else
            mvwaddch(win, sy, sx, ACS_HLINE);
    } else {
        mvwaddch(win, sy, sx, ' ');
    }

    mvwaddch (win, y, x, 'O');
    wattrset(win, COLOR_PAIR (1));
}

static void print_hints(int y, int x, int res[], int cnt, WINDOW *win) {
    int i, xpos;

    for (i = 0; i < cnt; i++) {
        xpos = x + i * (i == 0 ? 0 : 4);
        mvwprintw(win, y, xpos, "%3d", res[i]);
        if (i < (cnt - 1)) mvwaddch(win, y, xpos + 3, ',');
    }
}

static void clear_hints() {
    int i;
    print_map(nlab, mlab, win);
    print_player(x, y, xo, yo, win);
    print_ghosts(win);
    for (i = 22; i < COLS; i++) mvwaddch(win, 1, i, ' '); // obrisi hintove / brojeve
    wrefresh(win);
}

static void print_route(int lin, int col, int pos, int row, WINDOW *win) {
    int i, j, n, r, min, max;
    int a, b;
    int *res = (int *) malloc(sizeof(int) * lin);
    get_res(labir, res, pos, row, col, lin);

    wattrset(win, A_BOLD | COLOR_PAIR (3));
    print_hints(1, 22, res, lin - row, win);

    if (!win || row > lin) return; // fail

    //getbegyx(win,yo,xo);
    getmaxyx(win, b, a);

    n = (lin > b - 3 ? b - 3 : lin);


    for (i = 0, r = row * 2 + 2; i < n - row; i++, r += 2) {
        if (res[i] < 0) {
            min = pos + res[i];
            max = pos - 1;
            mvwaddch(win, r, max + 1, ACS_BTEE);
        } else {
            min = pos + 1;
            max = pos + res[i];
            mvwaddch(win, r, min - 1, ACS_BTEE);
        }
        pos += res[i];

        for (j = min; j <= max; j++) {
            mvwaddch(win, r, j, ACS_HLINE);
        }

        mvwaddch(win, r, pos, (i - 1 > 0 && labir[i - 1][pos]) ? ACS_VLINE : ACS_TTEE);
        mvwaddch(win, r + 1, pos, ACS_VLINE);
        mvwaddch(win, r + 2, pos, (i + 1 < n && labir[i + 1][pos]) ? ACS_VLINE : ACS_BTEE);
    }

    free(res);
    wrefresh(win);
}

static float get_skill_factor() {
    if (skill_level > ARRAY_SIZE(skill_factors) || skill_level < 1)
        return skill_factors[0];
    return skill_factors[skill_level - 1];
}


static void init_ghosts(int lines, int cols) {
    int i;
    sghosts *ghost;

    free_ghosts();

    if ((ghosts = (sghosts **) calloc(sizeof(sghosts *), lines - 1)) == NULL) // prvi red nema duha
        fatal_error("Cannot allocate memory for labir:ghosts");

    srand((unsigned) time(NULL));

    // place ghosts
    for (i = 0; i < lines - 1; i++) {
        if ((ghosts[i] = (sghosts *) calloc(sizeof(sghosts), 1)) == NULL)
            fatal_error("Cannot allocate memory for labir:ghost");
        ghost = ghosts[i];
        ghost->pos = rand() % cols;
        ghost->ex_pos = 0;
        ghost->trend = (rand() % cols > cols / 2) ? 0 : 1;
        ghost->changes = 0;
    }
}

/**
 * Release ghosts.
 */
void free_ghosts() {
    int i;
    if (ghosts != NULL) {
        for (i = 0; i < nlab; i++) {
            if (ghosts[i] != NULL)
                free(ghosts[i]);
        }
        free(ghosts);
        ghosts = NULL;
    }
}

/**
 * Free up memory allocated for labyrinth.
 */
void free_labirint() {
    int i;
    if (labir != NULL) {
        for (i = 0; i < nlab; i++) {
            if (labir[i] != NULL)
                free(labir[i]);
        }
        free(labir);
        labir = NULL;
    }
}

// pomakni duhove za jedan korak u nasumicnom smjeru
// TODO: optimizacija
static void evolve_ghosts() {
    int rnd, i, res;
    srand((unsigned) time(NULL));
    sghosts *ghost;

    for (i = 0; i < nlab - 1; i++) {
        rnd = rand() % mlab;
        ghost = ghosts[i];
        ghost->ex_pos = ghost->pos;

        if (rnd < mlab / 2) {
            if (ghost->trend == 1) {
                if (ghost->changes > SKILL_LENGTH) {
                    res = -1;
                    ghost->changes = 1;
                    ghost->trend = -1;
                } else {
                    res = 1;
                    ghost->changes++;
                }
            } else {
                res = -1;
                ghost->changes++;
            }
        } else {
            if (ghost->trend == -1) {
                if (ghost->changes > SKILL_LENGTH) {
                    res = 1;
                    ghost->changes = 1;
                    ghost->trend = 1;
                } else {
                    res = -1;
                    ghost->changes++;
                }
            } else {
                res = 1;
                ghost->changes++;
            }
        }

        // provjera granica
        if (ghost->pos < 1) {
            ghost->trend = 1;
            ghost->changes = 1;
            res = 1;
        } else if (ghost->pos >= mlab - 1) {
            ghost->trend = -1;
            ghost->changes = 1;
            res = -1;
        }

        // napokon
        ghost->pos += res;
    }
}

// ispisi duhove na win
static void print_ghosts(WINDOW *win) {
    int i, r;

    if (skill_level > 1) {
        wattrset(win, A_BOLD | COLOR_PAIR (4));

        for (i = 0, r = 4; i < nlab - 1; i++, r += 2) {
            mvwaddch(win, r, (ghosts[i])->ex_pos, ' ');
            mvwaddch(win, r, (ghosts[i])->pos, '@');
        }

        wattrset(win, A_BOLD | COLOR_PAIR (1));
    }
}

/**
* returns true if ghost hit the player
*/
static bool check_ghosts() {
    int i, r;
    for (i = 0, r = 4; i < nlab - 1; i++, r += 2) {
        if ((ghosts[i])->pos == x && r == y)
            return TRUE;
    }
    return FALSE;
}

void clean_game() {
    // destroy game window
    if (win != NULL)
        delwin(win);

    //free mem
    free_labirint();
    free_ghosts();
}

/** pokreni igricu u zadanim okvirima
*/
void init_game(int p_lines, int p_cols) {
    chtype button = 0;
    float _skill_factor = get_skill_factor();
    clock_t ghost_update = clock();
    int i;

    clear();
    win = newwin(p_lines, p_cols, 0, 0);
    keypad(win, TRUE);
    cbreak();
    nodelay(win, TRUE);

    nlab = p_lines / 2 - 3;
    mlab = p_cols;

    distance = 0;
    hint = 0;
    show_trail = 1;

    generate_labirint(nlab, mlab, nrupa);
    print_map(nlab, mlab, win);
    init_ghosts(nlab, mlab);

    wattrset(win, A_BOLD | COLOR_PAIR (5));
    x = mlab / 2;
    y = 2;
    xo = x;
    yo = y;
    mvwprintw(win, 0, 0, " \'%c\' for Quit | <- left -> right \'%c\' advance", QUIT, ADVANCE);
    wattrset(win, COLOR_PAIR (1));
    update_footer(p_lines);

    print_player(x, y, xo, yo, win);
    mvwprintw(win, 1, 0, "Putnik pos: %03d %03d", y, x);

    for (i = 0; i < COLS; i++) mvwaddch(win, p_lines - 2, i, '-');
    i = 0;
    wrefresh(win);

    do {
        button = wgetch(win);

        xo = x;
        yo = y;
        switch (button) {
            case LEFT:
                if (x == 0)
                    x = 0;
                else {
                    x--;
                    distance++;
                }
                break;
            case RIGHT:
                if (x == mlab - 1)
                    x = mlab - 1;
                else {
                    x++;
                    distance++;
                }
                break;
            case KEY_DOWN:
            case ADVANCE:
                if (y >= 2 * nlab + 2)
                    y = 2 * nlab + 2;
                else if (labir[i][x] == 0)
                    y = y;
                else {
                    y += 2;
                    i++;
                    distance++;
                    if (hint) {
                        clear_hints();
                        print_route(nlab, mlab, xo, (y - 2) / 2, win);
                    }
                }
                break;

            case KEY_HINT: //key h
                // show route
                if (!hint) {
                    print_route(nlab, mlab, xo, (y - 2) / 2, win);
                    hint = 1;
                } else {
                    clear_hints();
                    hint = 0;
                }
                update_footer(p_lines);
                break;

            case KEY_TRAIL: //key t
                if (!show_trail) {
                    show_trail = 1;
                } else {
                    clear_hints();
                    show_trail = 0;
                }
                update_footer(p_lines);
                break;

            case KEY_SKILL: //key s
                if (skill_level >= ARRAY_SIZE(skill_factors)) {
                    skill_level = 1;
                    clear_hints(); // ocisti retke
                } else
                    skill_level++;
                _skill_factor = get_skill_factor();
                update_footer(p_lines);
                break;

            default:
                break;
        }
        //mvprintw(21,0, "time: %20g",clock());
        if (_skill_factor > 0 && (ghost_update + _skill_factor * CLOCKS_PER_SEC) < clock()) {
            evolve_ghosts();
            print_ghosts(win);
            ghost_update = clock();
        }

        // check if catched by ghost
        if (_skill_factor > 0 && check_ghosts()) {
            break;
        }

        print_player(x, y, xo, yo, win);
        mvwprintw(win, 1, 0, "Putnik pos: %03d %03d", y, x);
        mvwprintw(win, p_lines - 1, p_cols - 14, "distance: %3d", distance);
        wmove(win, 0, 0);
        wrefresh(win);

    } while (button != QUIT && y != 2 * nlab + 2);

    if (y == 2 * nlab + 2) {
        print_dialog("END OF THE ROUTE!", COLOR_PAIR(3), 45, 6);
    } else if (button != QUIT) {
        // assume game over
        print_dialog("GAME OVER!", COLOR_PAIR(4), 45, 6);
    }
}

static void update_footer(int p_lines) {
    wattrset(win, COLOR_PAIR (1));
    mvwprintw(win, p_lines - 1, 0, "[H]int %3s | [T]rail %3s | [S]kill %1d",
              hint ? STRING_ON : STRING_OFF,
              show_trail ? STRING_ON : STRING_OFF,
              skill_level
    );
}

void print_dialog(char *msg, chtype color, int width, int height) {
    WINDOW *cwin = newwin(height, width, LINES / 2 - height / 2, COLS / 2 - width / 2);
    wattrset(cwin, A_BOLD | color);
    box(cwin, 0, 0);
    print_in_middle(cwin, 1, 0, width, msg, A_BOLD | color);
    wattrset(cwin, ~A_BOLD & color);
    print_in_middle(cwin, height - 2, 0, width, "Press <ENTER> to quit", COLOR_PAIR(1));
    //wmove(cwin, 0,0);
    while (wgetch(cwin) != 10);
    delwin(cwin);
}
