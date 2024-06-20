#include "lcd.h"

struct lcd_device* newScreen (const char* path) {
    struct lcd_device* screen = (struct lcd_device*)malloc(sizeof(struct lcd_device));
    if (screen==NULL)  {
        perror("insufficient satck memory");
        return NULL;
    }

    //access screen
    int fd = open(path,O_RDWR|O_NONBLOCK);
    if (fd==-1) {
        perror("screen not available");
        goto open_error;
    }

    //get screen info
    struct fb_var_screeninfo s_info;
    if(ioctl(fd,FBIOGET_VSCREENINFO,&s_info)==-1) {
        perror("failed to get screen info");
        goto general_error;
    }

    //put said info into struct
    screen->w = s_info.xres;
    screen->h = s_info.yres;
    screen->pixel_size = s_info.bits_per_pixel/8;
    screen->screen_size = screen->w*screen->h;

    //map the pointer and get the path to screen file
    screen->ptr = mmap(NULL,screen->screen_size*screen->pixel_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    screen->path = (char*)malloc(sizeof(char)*(strlen(path)+1));
    if(screen->path == NULL) {
        perror("insufficient satck memory");
        goto general_error;
    }
    strcpy(screen->path,path);

    //put the functions in
    screen->clear = clearScreen;
    screen->remove = removeScreen;
    screen->paint = paintMap;

    //if all is good
    close(fd);
    return screen;

general_error:
    close(fd);
open_error:
    free(screen);
    return NULL;
}

static void removeScreen (struct lcd_device* screen) {
    munmap(screen->path,screen->screen_size*4);
    free(screen->path);
    free(screen);
}

static void clearScreen (struct lcd_device* screen,int color) {
    for (int i=0;i<screen->screen_size;i++)
        memcpy(screen->ptr+i*screen->pixel_size,&color,screen->pixel_size);
}


static bool paintMap (struct lcd_device* screen,void* map,int width,int height,int x0,int y0) {
    if (screen==NULL) return false;

    //adjust the paint size if about to be out of range
    if (x0+width>screen->w) width=screen->w-x0;
    if (y0+height>screen->h) height=screen->h-y0;

    //now paint it
    for (int i=0;i<height;i++)
        memcpy(screen->ptr+screen->pixel_size*(screen->w*(y0+i)+x0),map+width*i*screen->pixel_size,width*screen->pixel_size);

    //if all is good
    return true;
}