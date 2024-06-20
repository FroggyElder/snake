#include "touch_screen.h"

int main () {
    struct tscreen* screen = TScreen_new("/dev/input/event0");

    while (1) {
        int beh = screen->getAction(screen);
        switch (beh)
        {
            case TS_TAP:
                printf ("Tap at x:%d y:%d\n",screen->status->x,screen->status->y);
                break;
            case TS_UP:
                printf ("Slide up\n");
                break;
            case TS_DOWN:
                printf ("Slide down\n");
                break;
            case TS_LEFT:
                printf ("Slide left\n");
                break;
            case TS_RIGHT:
                printf ("Slide right\n");
                break;
            default:
                break;
        }
    }

    screen->remove(screen);
    return 0;
}