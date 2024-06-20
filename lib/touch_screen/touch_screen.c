#include "touch_screen.h"

struct tscreen* TScreen_new (char* path) {
    //make space
    struct tscreen* new_screen = (struct tscreen*)malloc(sizeof(struct tscreen));
    if (new_screen==NULL) {
        perror("failed to make space for touch screen");
        return NULL;
    }

    //initiate status
    new_screen->status = (struct tscreen_status*)malloc(sizeof(struct tscreen_status));
    new_screen->status->pressed = false;
    new_screen->status->x = -1;
    new_screen->status->y = -1;
    new_screen->fd = open(path,O_RDONLY);

    //allocate functions
    new_screen->remove = TScreen_destory;
    new_screen->update = updateStatus;
    new_screen->getAction = getAction;

    return new_screen;
}

static bool TScreen_destory (struct tscreen* touch_screen) {
    if (touch_screen==NULL) return false;

    close(touch_screen->fd);
    free(touch_screen->status);
    touch_screen->status = NULL;
    free(touch_screen);
    return true;
}

static int updateStatus (struct tscreen* touch_screen) {
    //check if touch screen is avaiable for read
    if(touch_screen==NULL){
        perror("illigal screen");
        goto init_error;
    }
    if (touch_screen->status==NULL) {
        perror("Screen info corrupted, destorying");
        TScreen_destory (touch_screen);
        goto init_error;
    }

    struct input_event input_struct;

    //loop read the struct tscreen till info appears.
    while (1) {
        if(read(touch_screen->fd,&input_struct,sizeof(struct input_event))<0) {
            perror("failed to access touch screen.");
            goto general_error;
        }

        if(input_struct.type==EV_KEY&&input_struct.code==BTN_TOUCH){
            if (input_struct.value==0) touch_screen->status->pressed = false;
            if (input_struct.value>0) touch_screen->status->pressed = true;
            break;
        }
    
        if(input_struct.type==EV_ABS) {
            if (input_struct.code==ABS_X) touch_screen->status->x = input_struct.value*X_TOUCH_TO_LCD;
            if (input_struct.code==ABS_Y) touch_screen->status->y = input_struct.value*Y_TOUCH_TO_LCD;
            break;
        }
    }

    //if all is good
    return 0;

general_error:
init_error:
    return -1;
}

static int getAction (struct tscreen* touch_screen) {

    while (!touch_screen->status->pressed) 
    {
        touch_screen->update(touch_screen);
    }
    int x0 = touch_screen->status->x;
    int y0 = touch_screen->status->y;

    while (touch_screen->status->pressed) 
    {
        touch_screen->update(touch_screen);
    }
    int x1 = touch_screen->status->x;
    int y1 = touch_screen->status->y;

    if (x0==x1&&y0==y1) return TS_TAP;

    int x_drift = x1>x0 ? x1-x0 : x0 - x1;
    int y_drift = y1>y0 ? y1-y0 : y0 - y1;

    if (x_drift>y_drift) return x1>x0 ? TS_RIGHT : TS_LEFT;
    else return y1>y0 ? TS_DOWN : TS_UP;
}