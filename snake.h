#ifndef _snake_H
#define _snake_H

#include "touch_screen.h"
#include "lcd.h"
#include "bmp_read.h"
#include "font.h"

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define LCD0 "/dev/fb0"
#define TS0 "/dev/input/event0"

#define ARGB_BLACK 0xff000000
#define ARGB_WHITE 0xffffffff
#define ARGB_GREEN 0xff66ff33
#define ARGB_YELLOW 0xffccff33
#define ARGB_PINK 0xffffccff

#define BOARD_WIDTH 40
#define BOARD_HEIGHT 24
enum board_tile {
    BT_EMPTY=0,BT_FOOD,BT_SNAKE,BT_WALL
};

void boardClear ();
void boardRePaint ();

#define PLAY_BUTTON_PATH "./assets/play.bmp"
#define EXIT_BUTTON_PATH "./assets/exit.bmp"
#define BUTTON_1_WHITE "./assets/1x_white.bmp"
#define BUTTON_1_YELLOW "./assets/1x_yellow.bmp"
#define BUTTON_2_WHITE "./assets/1.5x_white.bmp"
#define BUTTON_2_YELLOW "./assets/1.5x_yellow.bmp"
#define BUTTON_3_WHITE "./assets/2x_white.bmp"
#define BUTTON_3_YELLOW "./assets/2x_yellow.bmp"

#define BUTTON_SIZE 96

#define DEBUG_INFO printf("[%s:%d] in %s\n",__func__,__LINE__,__FILE__)

struct button {
    unsigned int* pixels;
    int x;
    int y;
    int w;
    int h;
    int isActive;
};
struct button* buttonInit (void* data,int w,int h,int x,int y);
void buttonDestroy (struct button* button);
bool buttonAdd (struct button* button,struct lcd_device* lcd);
bool buttonRemove (struct button* button,struct lcd_device* lcd);
bool buttonReplace (struct button* button,void* new_data);
bool isInButton (struct tscreen* ts,struct button* button);

void printText (char* text,font* font,int len,int size,int x,int y);
void printCircle (int r,int x,int y,unsigned int c);
void clearSquare (int w,int h,int x,int y);

enum snake_heading {
    H_LEFT = 1,H_RIGHT,H_UP,H_DOWN
};
struct snake_node {
    int x;
    int y;
    struct snake_node* next;
    struct snake_node* prev;
};
struct snake {
    struct snake_node* head;
    int heading;
};
void snakeInit ();
void snakeKill ();
void snakeReset ();
struct snake_node* snakeNewNode (int x,int y);
void snakeHeadInsert (int x,int y);
void snakeTailRemove ();
void snakePrint ();
void* snakeTick (void* whatever);

void addFood (int num);

bool gameOverScreen ();
bool startScreen ();
bool pauseScreen ();

#endif /*_snake_H*/