/*==============================================================================
** ui.c -- a tetris game in tty.
**
** MODIFY HISTORY:
**
** 2011-09-29 wdf Create.
==============================================================================*/

#include <stdio.h>
#include <string.h>  /* strcmp() */
#include <fcntl.h>   /* open() read() write() close() */
#include <ctype.h>   /* isprint() */
#include "msg.h"

/*==============================================================================
  show config
==============================================================================*/
#define serial_printf   printf
#define serial_putc     putchar
#define serial_read     read
/*==============================================================================
  show config
==============================================================================*/
#define NODE_WIDTH          2    /* char too thin, so a node occupy two char */
#define NODE_ELEMENT        "__" /* node's chars */
#define CURSOR_COL_START    10   /* the fisrt node cursor column */
#define CURSOR_ROW_START    6    /* the fisrt node cursor row */
#define NODE_LR_NUM         10   /* left -- right node number */
#define NODE_TB_NUM         20   /* top -- bottom node number */
#define EXTERN_WIDTH        10
/*==============================================================================
  VT100+ Color codes
==============================================================================*/
#define COLOR_FG_BLACK  "\033[30m"             /*  ºÚ   ×Ö                    */
#define COLOR_FG_RED    "\033[31m"             /*  ºì   ×Ö                    */
#define COLOR_FG_GREEN  "\033[32m"             /*  ÂÌ   ×Ö                    */
#define COLOR_FG_YELLOW "\033[33m"             /*  »Æ   ×Ö                    */
#define COLOR_FG_BLUE   "\033[34m"             /*  À¶   ×Ö                    */
#define COLOR_FG_MAGEN  "\033[35m"             /*  Æ·ºì ×Ö                    */
#define COLOR_FG_CYAN   "\033[36m"             /*  Çà   ×Ö                    */
#define COLOR_FG_WHITE  "\033[37m"             /*  °×   ×Ö                    */
#define COLOR_FG_DEF    "\033[0m"              /*  Ä¬ÈÏÑÕÉ«                   */

#define COLOR_BG_BLACK  "\033[40m"             /*  ºÚ   µ×                    */
#define COLOR_BG_RED    "\033[41m"             /*  ºì   µ×                    */
#define COLOR_BG_GREEN  "\033[42m"             /*  ÂÌ   µ×                    */
#define COLOR_BG_YELLOW "\033[43m"             /*  »Æ   µ×                    */
#define COLOR_BG_BLUE   "\033[44m"             /*  À¶   µ×                    */
#define COLOR_BG_MAGEN  "\033[45m"             /*  Æ·ºì µ×                    */
#define COLOR_BG_CYAN   "\033[46m"             /*  Çà   µ×                    */
#define COLOR_BG_WHITE  "\033[47m"             /*  °×   µ×                    */

#define CLEAR_SCREEN    "\033[H\033[0J"        /*  ÇåÆÁ                       */
#define HIDE_CURSOR     "\033[?25l"            /*  Òþ²Ø¹â±ê                   */
#define SHOW_CURSOR     "\033[?25h"            /*  ÏÔÊ¾¹â±ê                   */
/*===============================================================================
  KeyBoard Strings
===============================================================================*/
#define KEY_ESC         "\033"
#define KEY_UP          "\033[A"
#define KEY_DOWN        "\033[B"
#define KEY_RIGHT       "\033[C"
#define KEY_LEFT        "\033[D"
#define KEY_TAB         "\t"
#define KEY_BOTTOM      " "         /* fix block at bottom */
#define KEY_HOLD        "h"         /* freeze tetris */
#define KEY_AUTO        "`"         /* the key below [ESC] and left [1] */
/*===============================================================================
  the Tetris core support only API function
===============================================================================*/
void tetris_run (int column_num, int row_num,
                 void (*show_node)(int x, int y, int show),
                 void (*show_score)(int type, int value),
                 int  (*input_msg)(void));

/*==============================================================================
 * - _draw_frame()
 *
 * - the frame looks like this:
 *   +------------------+-----+
 *   |                  |     |
 *   +------------------+     |
 *   |                  |     |
 *   |                  |     |
 *   |                  |     |
 *   |                  |     |
 *   |                  |     |
 *   |                  |     |
 *   |                  |     |
 *   |                  |     |
 *   |                  |     |
 *   +------------------+-----+
 */
static void _draw_frame (void)
{
    int i;

    /*
     * draw the hat, this area is for show score
     */
    
    /* move cursor to hat left top  */
    serial_printf ("\033[%d;%dH", CURSOR_ROW_START - 3, CURSOR_COL_START - 1);
    /* +--------------+ */
    /* |              | */
    serial_putc ('+');
    for (i = 0; i < NODE_LR_NUM * NODE_WIDTH; i++) serial_putc ('-');
    serial_putc ('+');

    serial_printf ("\033[%d;%dH", CURSOR_ROW_START - 2, CURSOR_COL_START - 1);
    serial_putc ('|');
    serial_printf ("\033[%d;%dH", CURSOR_ROW_START - 2, CURSOR_COL_START + NODE_LR_NUM * NODE_WIDTH);
    serial_putc ('|');
    printf("\r\n");

    /*
     * draw the body, this area is for game
     */

    /* move cursor to left top */
    serial_printf ("\033[%d;%dH", CURSOR_ROW_START - 1, CURSOR_COL_START - 1);
    /* +--------------+ */
    serial_putc ('+');
    for (i = 0; i < NODE_LR_NUM * NODE_WIDTH; i++) serial_putc ('-');
    serial_putc ('+');
    printf("\r\n");

    /* draw left & right wall */
    for (i = 0; i < NODE_TB_NUM; i++) {
        serial_printf ("\033[%d;%dH", CURSOR_ROW_START+i, CURSOR_COL_START - 1);
        serial_putc ('|');
        serial_printf ("\033[%d;%dH", CURSOR_ROW_START+i, CURSOR_COL_START + NODE_LR_NUM * NODE_WIDTH);
        serial_putc ('|');
    }
    printf("\r\n");

    /* move cursor to left bottom */
    serial_printf ("\033[%d;%dH", CURSOR_ROW_START + NODE_TB_NUM, CURSOR_COL_START - 1);
    /* +--------------+ */
    serial_putc ('+');
    for (i = 0; i < NODE_LR_NUM * NODE_WIDTH; i++) serial_putc ('-');
    serial_putc ('+');
    printf("\r\n");

    /*
     * draw the extern, this area is for show No., Next block, Speed,Time
     */

    serial_printf ("\033[%d;%dH", CURSOR_ROW_START - 3, CURSOR_COL_START + NODE_LR_NUM * NODE_WIDTH + 1);
    /* ---------+ */
    for (i = 0; i < EXTERN_WIDTH; i++) serial_putc ('-');
    serial_putc ('+');
    serial_printf ("\033[%d;%dH", CURSOR_ROW_START + NODE_TB_NUM, CURSOR_COL_START + NODE_LR_NUM * NODE_WIDTH + 1);
    for (i = 0; i < EXTERN_WIDTH; i++) serial_putc ('-');
    serial_putc ('+');

    /* draw extern wall */
    for (i = 0; i < NODE_TB_NUM + 2; i++) {
        serial_printf ("\033[%d;%dH", CURSOR_ROW_START - 2 + i, CURSOR_COL_START + NODE_LR_NUM * NODE_WIDTH + EXTERN_WIDTH + 1);
        serial_putc ('|');
    }
    printf("\r\n");


    /* draw extern separators */
    serial_printf ("\033[%d;%dH", CURSOR_ROW_START + 4,  CURSOR_COL_START + NODE_LR_NUM * NODE_WIDTH + 1);
    for (i = 0; i < EXTERN_WIDTH; i++) serial_putc ('-');
    serial_printf ("\033[%d;%dH", CURSOR_ROW_START + 7,  CURSOR_COL_START + NODE_LR_NUM * NODE_WIDTH + 1);
    for (i = 0; i < EXTERN_WIDTH; i++) serial_putc ('-');
    serial_printf ("\033[%d;%dH", CURSOR_ROW_START + 10, CURSOR_COL_START + NODE_LR_NUM * NODE_WIDTH + 1);
    for (i = 0; i < EXTERN_WIDTH; i++) serial_putc ('-');
    printf("\r\n");
}

/*======================================================================
  rank data struct
======================================================================*/
#define RANK_NAME_LEN  4
#define RANK_LIST_CNT  7
typedef struct rank_data {
    char  name[RANK_NAME_LEN];
    int   score;
} RANK_DATA;
RANK_DATA   _G_rank_list[RANK_LIST_CNT];
int         _G_score;
/*==============================================================================
 * - _print_rank()
 *
 * - read file and show ranking list
 */
static void _print_rank (void)
{
    int i;
    int fd;
    RANK_DATA rank;
    ssize_t nread;
    int start_col;
    int start_row;

    start_col = CURSOR_COL_START + (NODE_LR_NUM * NODE_WIDTH) + 2;
    start_row = CURSOR_ROW_START + 11;

    /* set background & foreground color */
    serial_printf (COLOR_BG_BLACK);
    serial_printf (COLOR_FG_YELLOW);

    /* move cursor */
    serial_printf ("\033[%d;%dH", start_row, start_col);
    serial_printf ("Rank");

    fd = open("rank", O_RDWR | O_CREAT, 0666);
    if (fd < 0) {
        return;
    }
    
    start_row += 2;

    /* print rank list */
    for (i = 0; i < RANK_LIST_CNT; i++) {
        nread = read(fd, &rank, sizeof(RANK_DATA));
        if (nread == sizeof(RANK_DATA)) {

            if (rank.score == 0) {
                break;
            }

            /* move cursor */
            serial_printf ("\033[%d;%dH", start_row, start_col);
            serial_printf ("%s: %d", rank.name, rank.score);

            _G_rank_list[i] = rank;

            start_row++;
        } else {
            break;
        }
    }

    close(fd);
}
/*==============================================================================
 * - _write_rank()
 *
 * - read file and write ranking list
 */
static void _write_rank (void)
{
    int rank_idx;
    int i;
    int fd;
    int start_col;
    int start_row;
    char ch;
    int  nread;

    for (rank_idx = 0; rank_idx < RANK_LIST_CNT; rank_idx++) {
        if (_G_score > _G_rank_list[rank_idx].score) {
            break;
        }
    }

    if (rank_idx == RANK_LIST_CNT) {
        return;
    }

    for (i = RANK_LIST_CNT - 1; i > rank_idx; i--) {
        _G_rank_list[i] = _G_rank_list[i - 1];
    }

    start_col = CURSOR_COL_START + ((NODE_LR_NUM * NODE_WIDTH) - 20) / 2;
    start_row = CURSOR_ROW_START + NODE_TB_NUM / 2;

    /* set background & foreground color */
    serial_printf (COLOR_BG_BLACK);
    serial_printf (COLOR_FG_YELLOW);

    /* move cursor */
    serial_printf ("\033[%d;%dH", start_row, start_col);
    serial_printf ("Input Your Name: ");
    printf("\r\n");

    start_col += 17;

    /* set background & foreground color */
    serial_printf (COLOR_BG_BLACK);
    serial_printf (COLOR_FG_RED);

    i = 0;
    while (i < RANK_NAME_LEN - 1) {
        nread = serial_read (STD_IN, &ch, 1);
        if (nread <= 0) {
            break;
        }
        if (ch == '\r') {
            break;
        }
        if (isprint(ch)) {
            _G_rank_list[rank_idx].name[i] = ch;
            printf("\r\n");
            serial_printf ("\033[%d;%dH", start_row, start_col);
            serial_printf("%c\r\n", ch);
            i++;
            start_col++;
        }
    }
    _G_rank_list[rank_idx].name[i] = '\0';
    _G_rank_list[rank_idx].score = _G_score;

    fd = open("rank", O_RDWR | O_CREAT, 0666);
    if (fd < 0) {
        return;
    }

    for (i = 0; i < RANK_LIST_CNT; i++) {
        write(fd, &_G_rank_list[i], sizeof(RANK_DATA));
    }
    close(fd);

    /* set background & foreground color */
    serial_printf (COLOR_BG_BLACK);
    serial_printf (COLOR_FG_YELLOW);

    start_col = CURSOR_COL_START + ((NODE_LR_NUM * NODE_WIDTH) - 16) / 2;
    start_row += 2;
    serial_printf ("\033[%d;%dH", start_row, start_col);
    serial_printf("Congratulations!\r\n");

    start_col = CURSOR_COL_START + ((NODE_LR_NUM * NODE_WIDTH) - 3) / 2;
    start_row++;
    serial_printf ("\033[%d;%dH", start_row, start_col);
    serial_printf("%s\r\n", _G_rank_list[rank_idx].name);

    start_col = CURSOR_COL_START + ((NODE_LR_NUM * NODE_WIDTH) - 13) / 2;
    start_row++;
    serial_printf ("\033[%d;%dH", start_row, start_col);
    serial_printf("Your are No.%d\r\n", rank_idx);
    printf("\r\n");
    getchar();
}

/*==============================================================================
 * - _draw_node()
 *
 * - draw the (x, y) position node, it looks like this:
 *   __
 */
static void _draw_node (int x, int y, int color)
{
    /* move cursor */
    serial_printf ("\033[%d;%dH", CURSOR_ROW_START + y, CURSOR_COL_START + (x * NODE_WIDTH));

    /* set foreground color as BLACK */
    serial_printf (COLOR_FG_BLACK);

    /* set background color */
    serial_printf ("\033[%dm", 40 + color);

    /* show or erase node */
    serial_printf (NODE_ELEMENT);
    printf("\r\n");
}

/*==============================================================================
 * - _draw_msg()
 *
 * - draw the tetris message, it looks like this:
 */
static void _draw_msg (int type, int value)
{
    int start_col;
    int start_row;

    /* set background & foreground color */
    serial_printf (COLOR_BG_BLACK);
    serial_printf (COLOR_FG_YELLOW);

    switch (type) {
    case MSG_SHOW_SCORE:
        start_col = CURSOR_COL_START + ((NODE_LR_NUM * NODE_WIDTH) - 10) / 2;
        start_row = CURSOR_ROW_START - 2;

        /* move cursor */
        serial_printf ("\033[%d;%dH", start_row, start_col);
        serial_printf ("Score: %d", value);

        _G_score = value;
        break;

    case MSG_SHOW_COUNT:
        start_col = CURSOR_COL_START + (NODE_LR_NUM * NODE_WIDTH) + 2;
        start_row = CURSOR_ROW_START - 2;

        /* move cursor */
        serial_printf ("\033[%d;%dH", start_row, start_col);
        serial_printf ("No.:%d", value);
        break;

    case MSG_SHOW_SPEED:
        start_col = CURSOR_COL_START + (NODE_LR_NUM * NODE_WIDTH) + 2;
        start_row = CURSOR_ROW_START + 5;

        /* move cursor */
        serial_printf ("\033[%d;%dH", start_row, start_col);
        serial_printf ("Speed");

        start_col += 2;
        start_row = CURSOR_ROW_START + 6;
        serial_printf ("\033[%d;%dH", start_row, start_col);
        serial_printf ("%d", value);
        break;

    case MSG_SHOW_TIME:
        start_col = CURSOR_COL_START + (NODE_LR_NUM * NODE_WIDTH) + 2;
        start_row = CURSOR_ROW_START + 8;

        /* move cursor */
        serial_printf ("\033[%d;%dH", start_row, start_col);
        serial_printf ("Time");

        start_row = CURSOR_ROW_START + 9;
        serial_printf ("\033[%d;%dH", start_row, start_col);
        if (value/3600 == 0) {
            serial_printf ("%02d:%02d", value%3600/60, value%60);
        } else {
            serial_printf ("%02d:%02d:%02d", value/3600, value%3600/60, value%60);
        }
        break;

    default:
        break;
    }

    printf("\r\n");
}

/*==============================================================================
 * - _input_msg()
 *
 * - get user input key.
 */
static int _input_msg (void)
{
    int  ret;
    char key[4];
    int  nread;

    nread = serial_read (STD_IN, key, 3);
    key[nread] = '\0';

    if (strcmp (key, KEY_ESC) == 0) {           /* Esc: quit */
        ret = TETRIS_MSG_QUIT;
    } else if (strcmp (key, KEY_UP) == 0) {     /* Up */
        ret = TETRIS_MSG_TURN;
    } else if (strcmp (key, KEY_TAB) == 0) {    /* Tab */
        ret = TETRIS_MSG_TURN;
    } else if (strcmp (key, KEY_DOWN) == 0) {   /* Down */
        ret = TETRIS_MSG_DOWN;
    } else if (strcmp (key, KEY_RIGHT) == 0) {  /* Right */
        ret = TETRIS_MSG_RIGHT;
    } else if (strcmp (key, KEY_LEFT) == 0) {   /* Left */
        ret = TETRIS_MSG_LEFT;
    } else if (strcmp (key, KEY_AUTO) == 0) {   /* '`': auto */
        ret = TETRIS_MSG_AUTO;
    } else if (strcmp (key, KEY_BOTTOM) == 0) { /* ' ': bottom */
        ret = TETRIS_MSG_BOTTOM;
    } else if (strcmp (key, KEY_HOLD) == 0) {   /* 'h': hold on */
        ret = TETRIS_MSG_HOLD;
    } else {
        ret = TETRIS_MSG_NOP;
    }

    return ret;
}

/*==============================================================================
 * - tetris_main()
 *
 * - draw tetris frame and start tetris.
 */
void tetris_main (int argc, char **argv)
{
    INT  iInOldOption = OPT_TERMINAL;
    INT  iOutOldOption = OPT_TERMINAL;

    ioctl(STD_IN, FIOGETOPTIONS, &iInOldOption);
    ioctl(STD_IN, FIOSETOPTIONS, OPT_RAW);

    ioctl(STD_OUT, FIOGETOPTIONS, &iOutOldOption);
    ioctl(STD_OUT, FIOSETOPTIONS, OPT_RAW);

    /* clear screen and hide cursor */
    serial_printf (CLEAR_SCREEN);
    serial_printf (HIDE_CURSOR);

    /* draw frame */
    _draw_frame ();

    _print_rank();

    /* start tetris */
    tetris_run (NODE_LR_NUM, NODE_TB_NUM, _draw_node, _draw_msg, _input_msg);

    _write_rank();

    /* clear screen, show cursor and set cmd mode */
    serial_printf (COLOR_FG_WHITE);
    serial_printf (COLOR_BG_BLACK);
    serial_printf (CLEAR_SCREEN);
    serial_printf (SHOW_CURSOR);

    ioctl(STD_IN, FIOSETOPTIONS, iInOldOption);
    ioctl(STD_OUT, FIOSETOPTIONS, iOutOldOption);
}

/*==============================================================================
** FILE END
==============================================================================*/

