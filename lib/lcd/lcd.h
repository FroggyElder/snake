#ifndef _LCD_H_
#define _LCD_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <stdbool.h>

//struct to refer to a lcd device
struct lcd_device{
    //pointer to mmap
    void* ptr;
    //path of screen control file
    char* path;
    //screen size
    int screen_size;
    //screen width
    int w;
    //screen height
    int h;
    //bytes per pixel
    int pixel_size;

    void (* clear) (struct lcd_device* screen,int color);
    void (* remove) (struct lcd_device* screen);
    bool (* paint) (struct lcd_device* screen,void* map,int width,int height,int x0,int y0) ;
};

//create a new screen and return the pointer to the struct
struct lcd_device* newScreen (const char* path);

//remove the struct and cease controlling the screen
static void removeScreen (struct lcd_device* screen);

//fill the screen with 4byte RGB defined by color
static void clearScreen (struct lcd_device* screen,int color);

//paint screen with a series of data
static bool paintMap (struct lcd_device* screen,void* map,int width,int height,int x0,int y0);


#endif //_LCD_H_