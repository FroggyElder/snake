#include "snake.h"

static struct tscreen* ts = NULL;
static struct lcd_device* lcd = NULL;
static char s_moving = 1;
static struct snake* snake;
static struct score* score;
static char board[BOARD_WIDTH][BOARD_HEIGHT];
static int gg = 0;
static int game_speed = 2;


/*******************************board actions*******************************/

void* snakeTick (void* whatever) 
{
    int x,y;
    while (1) 
        {
start:
            if (!s_moving) {
                usleep(1000*100);
                continue;
            }

            //add food
            if(rand()%100<=10) addFood(1);

            //see where we are going
            x = snake->head->next->x;
            y = snake->head->next->y;
            switch (snake->heading) {
                case H_LEFT: --x; break;
                case H_RIGHT: ++x; break;
                case H_UP: --y; break;
                case H_DOWN: ++y; break;
            }

            //going out of border
            if (x<0||y<0||x>=BOARD_WIDTH||y>=BOARD_HEIGHT)
                goto game_over;

            //crashing on itself
                if (board[x][y]==BT_SNAKE) goto game_over;

            //if all is good
            switch (board[x][y]) 
            {
                //empty tile
                case BT_EMPTY:
                    //remove tail
                    clearSquare(20,20,20*snake->head->prev->x,20*snake->head->prev->y);
                    snakeTailRemove();

                    //add head
                    printCircle(10,20*x,20*y,ARGB_YELLOW);
                    snakeHeadInsert(x,y);
                    printCircle(10,20*snake->head->next->next->x,20*snake->head->next->next->y,ARGB_GREEN);
                    break;

                case BT_FOOD:
                    //add head
                    printCircle(10,20*x,20*y,ARGB_YELLOW);
                    snakeHeadInsert(x,y);
                    printCircle(10,20*snake->head->next->next->x,20*snake->head->next->next->y,ARGB_GREEN);
                    socreAdd(1);
                    break;

                case BT_WALL:
                    goto game_over;
                    break;
            }
            usleep(1000*500/game_speed);
        }

game_over:
    gg = 1;
    if (!gameOverScreen()) {
        lcd->clear(lcd,ARGB_BLACK);
        exit(0);
    }
    else {
        lcd->clear(lcd,ARGB_BLACK);
        boardClear();
        snakeReset();
        gg = 0;
        goto start;
    }
}

void boardClear () {
    for (int i=0;i<BOARD_WIDTH;i++)
        for (int j=0;j<BOARD_WIDTH;j++)
            board[i][j] = BT_EMPTY;
}

void addFood (int num) {
    for (int i=0;i<num;i++) {
        int x = random()%BOARD_WIDTH;
        int y = random()%BOARD_HEIGHT;

        if (!board[x][y]==BT_EMPTY) continue;

        board[x][y] = BT_FOOD;
        printCircle(10,20*x,20*y,ARGB_PINK);
    }
}

/*****************score and save file actions**********************/
void scoreInit () {
    FILE* save = fopen(SAVE_PATH,"r");
    int file_score = 0;
    if (fscanf(save,"highscore:%d",&file_score)!=1) {
        DEBUG_INFO;
        return;
    }

    score = (struct score*)malloc(sizeof(struct score));
    score->high = file_score;
    score->current = 0;

    fclose (save);
}

void saveSync () {
    FILE* save = fopen(SAVE_PATH,"r");
    int file_score = 0;
    if (fscanf(save,"highscore:%d",&file_score)!=1) {
        DEBUG_INFO;
        return;
    }
    fclose(save);

    score->high = INT_MAX(score->high,score->current);
    score->high = INT_MAX(file_score,score->high);
    
    save = fopen(SAVE_PATH,"w");
    fprintf (save,"highscore:%d",score->high);
    fclose(save);
}
void socrePrint () {
    char score_text[100];

    font* simfang = fontLoad("./assets/simfang.ttf");

    sprintf(score_text,"High score:%d",score->high);
    printText(score_text,simfang,30,strlen(score_text),(lcd->w-strlen(score_text)*15)/2,0);
    sprintf(score_text,"Score:%d",score->current);
    printText(score_text,simfang,30,strlen(score_text),(lcd->w-strlen(score_text)*15)/2,40);

    fontUnload(simfang);
    return;
}
void socreReset () {
    score->high = INT_MAX(score->high,score->current);
    score->current = 0;
}
void socreAdd (int val) {
    score->current+=val;
    score->high = INT_MAX(score->current,score->high);
}

struct snake_node* snakeNewNode (int x,int y) {
    struct snake_node* new_node = (struct snake_node*)malloc(sizeof(struct snake_node));
    new_node->x = x;
    new_node->y = y;
    new_node->next = new_node;
    new_node->prev = new_node;
    return new_node;
}

void snakeHeadInsert (int x,int y) {
    struct snake_node* new_node = (struct snake_node*)malloc(sizeof(struct snake_node));
    if (!new_node) {
        DEBUG_INFO;
        return;
    }

    board[x][y] = BT_SNAKE;
    new_node->x = x;
    new_node->y = y;

    new_node->prev = snake->head;
    new_node->next = snake->head->next;
    snake->head->next->prev = new_node;
    snake->head->next = new_node;
}

void snakeTailRemove () {
    if (snake->head->prev==snake->head) return;

    struct snake_node* tail = snake->head->prev;
    board[tail->x][tail->y] = BT_EMPTY;
    tail->prev->next = snake->head;
    snake->head->prev = tail->prev;
    free(tail);

}


void snakeInit () {
    snake = (struct snake*)malloc(sizeof(struct snake));
    snake->head = snakeNewNode(0,0);

    snakeHeadInsert(0,0);
    snakeHeadInsert(1,0);
    snakeHeadInsert(2,0);
    snakeHeadInsert(3,0);

    snake->heading = H_RIGHT;

    snakePrint();
}

void snakeReset () {
    while (snake->head->next!=snake->head) snakeTailRemove();
    snakeHeadInsert(0,0);
    snakeHeadInsert(1,0);
    snakeHeadInsert(2,0);
    snakeHeadInsert(3,0);
    snake->heading = H_RIGHT;

    snakePrint();
}
void snakeKill () {
    while (snake->head->next!=snake->head) snakeTailRemove();
    free(snake->head);
    free(snake);
}

void snakePrint () {
    struct snake_node* p = snake->head->next;
    printCircle(10,20*p->x,20*p->y,ARGB_YELLOW);
    for (p=p->next;p!=snake->head;p=p->next) {
        printCircle(10,20*p->x,20*p->y,ARGB_GREEN);
    }
}

struct button* buttonInit (void* data,int w,int h,int x,int y) {
    struct button* new_button = (struct button*)malloc(sizeof(struct button));
    if (new_button==NULL) {
        DEBUG_INFO;
        return NULL;
    }

    new_button->isActive = 0;
    new_button->pixels = data;
    new_button->w = w;
    new_button->h = h;
    new_button->x = x;
    new_button->y = y;

    return new_button;
}

void buttonDestroy (struct button* button) {
    free(button->pixels);
    free(button);
}

bool buttonAdd (struct button* button,struct lcd_device* lcd) {
    if (button==NULL||lcd==NULL||button->isActive) {
        DEBUG_INFO;
        return false;
    }

    button->isActive = 1;
    return lcd->paint(lcd,button->pixels,button->w,button->h,button->x,button->y);
}

bool buttonReplace (struct button* button,void* data) {
    if (button==NULL||lcd==NULL||!button->isActive||data==NULL) {
        DEBUG_INFO;
        return false;
    }

    free(button->pixels);
    button->pixels = data;
    return lcd->paint(lcd,button->pixels,button->w,button->h,button->x,button->y);
    return true;
}

bool buttonRemove (struct button* button,struct lcd_device* lcd) {
    if (button==NULL||lcd==NULL||!button->isActive) {
        DEBUG_INFO;
        return false;
    }

    unsigned int* blackPixels = (unsigned int*)calloc(button->h*button->w,sizeof(unsigned int));
    bool res = lcd->paint(lcd,blackPixels,button->w,button->h,button->x,button->y);
    button->isActive = 0;
    free(blackPixels);
    return res;
}

bool isInButton (struct tscreen* ts,struct button* button) {
    if (!button->isActive) return false;
    int  x0=button->x,y0=button->y,x1=button->x+button->w,y1=button->y+button->h;
    return ts->status->x>x0&&ts->status->x<x1&&ts->status->y>y0&&ts->status->y<y1;
}

void printText (char* text,font* font,int size,int len,int x,int y) {
    int w = size*len;
    int h = size;

    fontSetSize(font,size);
    bitmap* map = createBitmapWithInit(w,h,lcd->pixel_size,getColor(0,0,0,0));
    fontPrint(font,map,0,0,text,0xffffffff,0);

    lcd->paint(lcd,map->map,w,h,x,y);

    destroyBitmap(map);
}

void printCircle (int r,int x,int y,unsigned int c) {
    int d = r+r;
    unsigned int* p = lcd->ptr + 4*(y*lcd->w+x);

    for (int i=0;i<d;i++)
        for (int j=0;j<d;j++)
            if (abs(i-r)*abs(i-r)+abs(j-r)*abs(j-r) < r*r)
                *(p+(i*lcd->w+j)) = c;

}

void clearSquare (int w,int h,int x,int y) {
    unsigned int* p = lcd->ptr + 4*(y*lcd->w+x);

    for (int i=0;i<h;i++)
        for (int j=0;j<w;j++)
                *(p+(i*lcd->w+j)) = 0;
}

void boardRePaint () {
    lcd->clear(lcd,ARGB_BLACK);

    snakePrint();

    for (int i=0;i<BOARD_WIDTH;i++)
        for (int j=0;j<BOARD_HEIGHT;j++)
            if(board[i][j]==BT_FOOD) printCircle(10,20*i,20*j,ARGB_PINK);
    
}

bool gameOverScreen () {

    struct button* play_button = buttonInit
        (bmpToArgb(PLAY_BUTTON_PATH,NULL,NULL),BUTTON_SIZE,BUTTON_SIZE,(lcd->w-BUTTON_SIZE)/2,lcd->h/2);
    struct button* exit_button = buttonInit
        (bmpToArgb(EXIT_BUTTON_PATH,NULL,NULL),BUTTON_SIZE,BUTTON_SIZE,lcd->w-BUTTON_SIZE,0);
    struct button* spd_1_button = buttonInit
        (bmpToArgb(game_speed==2?BUTTON_1_YELLOW:BUTTON_1_WHITE,NULL,NULL),BUTTON_SIZE,BUTTON_SIZE,(lcd->w-BUTTON_SIZE)/2-BUTTON_SIZE-10,lcd->h/2+BUTTON_SIZE+10);
    struct button* spd_2_button = buttonInit
        (bmpToArgb(game_speed==3?BUTTON_2_YELLOW:BUTTON_2_WHITE,NULL,NULL),BUTTON_SIZE,BUTTON_SIZE,(lcd->w-BUTTON_SIZE)/2,lcd->h/2+BUTTON_SIZE+10);
    struct button* spd_3_button = buttonInit
        (bmpToArgb(game_speed==4?BUTTON_3_YELLOW:BUTTON_3_WHITE,NULL,NULL),BUTTON_SIZE,BUTTON_SIZE,(lcd->w-BUTTON_SIZE)/2+BUTTON_SIZE+10,lcd->h/2+BUTTON_SIZE+10);


    font* simfang = fontLoad("./assets/simfang.ttf");

    //Print gg screen
    lcd->clear(lcd,ARGB_BLACK);
    printText("你寄了",simfang,100,3,250,100);
    socrePrint();
    buttonAdd(play_button,lcd);
    buttonAdd(exit_button,lcd);
    buttonAdd(spd_1_button,lcd);
    buttonAdd(spd_2_button,lcd);
    buttonAdd(spd_3_button,lcd);
    
    bool res = false;
    while (1) {
        int action = ts->getAction(ts);
        if (action==TS_TAP) {
            if (isInButton(ts,play_button)) {
                res = true;
                break;
            }
            if (isInButton(ts,exit_button)) {
                res = false;
                break;
            }
            if (isInButton(ts,spd_1_button)) {
                if (game_speed==2) continue;
                switch (game_speed) {
                    case 3:
                        buttonReplace(spd_2_button,bmpToArgb(BUTTON_2_WHITE,NULL,NULL));
                        break;
                    case 4:
                        buttonReplace(spd_3_button,bmpToArgb(BUTTON_3_WHITE,NULL,NULL));
                        break;
                }
                buttonReplace(spd_1_button,bmpToArgb(BUTTON_1_YELLOW,NULL,NULL));
                game_speed = 2;
                continue;
            }
            if (isInButton(ts,spd_2_button)) {
                if (game_speed==3) continue;
                switch (game_speed) {
                    case 2:
                        buttonReplace(spd_1_button,bmpToArgb(BUTTON_1_WHITE,NULL,NULL));
                        break;
                    case 4:
                        buttonReplace(spd_3_button,bmpToArgb(BUTTON_3_WHITE,NULL,NULL));
                        break;
                }
                buttonReplace(spd_2_button,bmpToArgb(BUTTON_2_YELLOW,NULL,NULL));
                game_speed = 3;
                continue;
            }
            if (isInButton(ts,spd_3_button)) {
                if (game_speed==4) continue;
                switch (game_speed) {
                    case 2:
                        buttonReplace(spd_1_button,bmpToArgb(BUTTON_1_WHITE,NULL,NULL));
                        break;
                    case 3:
                        buttonReplace(spd_2_button,bmpToArgb(BUTTON_2_WHITE,NULL,NULL));
                        break;
                }
                buttonReplace(spd_3_button,bmpToArgb(BUTTON_3_YELLOW,NULL,NULL));
                game_speed = 4;
                continue;
            }
        }
    }

    saveSync ();
    socreReset ();
    buttonDestroy(spd_1_button);
    buttonDestroy(spd_2_button);
    buttonDestroy(spd_3_button);
    buttonDestroy(play_button);
    buttonDestroy(exit_button);
    fontUnload(simfang);
    return res;
}

bool startScreen () {
    //load font
    font* simfang = fontLoad("./assets/simfang.ttf");

    //initiate button
    struct button* play_button = buttonInit
        (bmpToArgb(PLAY_BUTTON_PATH,NULL,NULL),BUTTON_SIZE,BUTTON_SIZE,(lcd->w-BUTTON_SIZE)/2,lcd->h/2);
    struct button* exit_button = buttonInit
        (bmpToArgb(EXIT_BUTTON_PATH,NULL,NULL),BUTTON_SIZE,BUTTON_SIZE,lcd->w-BUTTON_SIZE,0);
    struct button* spd_1_button = buttonInit
        (bmpToArgb(BUTTON_1_YELLOW,NULL,NULL),BUTTON_SIZE,BUTTON_SIZE,(lcd->w-BUTTON_SIZE)/2-BUTTON_SIZE-10,lcd->h/2+BUTTON_SIZE+10);
    struct button* spd_2_button = buttonInit
        (bmpToArgb(BUTTON_2_WHITE,NULL,NULL),BUTTON_SIZE,BUTTON_SIZE,(lcd->w-BUTTON_SIZE)/2,lcd->h/2+BUTTON_SIZE+10);
    struct button* spd_3_button = buttonInit
        (bmpToArgb(BUTTON_3_WHITE,NULL,NULL),BUTTON_SIZE,BUTTON_SIZE,(lcd->w-BUTTON_SIZE)/2+BUTTON_SIZE+10,lcd->h/2+BUTTON_SIZE+10);

    //Print start screen
    lcd->clear(lcd,ARGB_BLACK);
    printText("贪吃蛇",simfang,100,3,250,100);
    socrePrint();
    buttonAdd(play_button,lcd);
    buttonAdd(exit_button,lcd);
    buttonAdd(spd_1_button,lcd);
    buttonAdd(spd_2_button,lcd);
    buttonAdd(spd_3_button,lcd);

    //wait for start
    bool ret = false;
    while (1) {
        int action = ts->getAction(ts);
        if (action==TS_TAP) {
            if (isInButton(ts,play_button)) {
                ret = true;
                break;
            }
            if (isInButton(ts,exit_button)) {
                ret = false;
                break;
            }
            if (isInButton(ts,spd_1_button)) {
                if (game_speed==2) continue;
                switch (game_speed) {
                    case 3:
                        buttonReplace(spd_2_button,bmpToArgb(BUTTON_2_WHITE,NULL,NULL));
                        break;
                    case 4:
                        buttonReplace(spd_3_button,bmpToArgb(BUTTON_3_WHITE,NULL,NULL));
                        break;
                }
                buttonReplace(spd_1_button,bmpToArgb(BUTTON_1_YELLOW,NULL,NULL));
                game_speed = 2;
                continue;
            }
            if (isInButton(ts,spd_2_button)) {
                if (game_speed==3) continue;
                switch (game_speed) {
                    case 2:
                        buttonReplace(spd_1_button,bmpToArgb(BUTTON_1_WHITE,NULL,NULL));
                        break;
                    case 4:
                        buttonReplace(spd_3_button,bmpToArgb(BUTTON_3_WHITE,NULL,NULL));
                        break;
                }
                buttonReplace(spd_2_button,bmpToArgb(BUTTON_2_YELLOW,NULL,NULL));
                game_speed = 3;
                continue;
            }
            if (isInButton(ts,spd_3_button)) {
                if (game_speed==4) continue;
                switch (game_speed) {
                    case 2:
                        buttonReplace(spd_1_button,bmpToArgb(BUTTON_1_WHITE,NULL,NULL));
                        break;
                    case 3:
                        buttonReplace(spd_2_button,bmpToArgb(BUTTON_2_WHITE,NULL,NULL));
                        break;
                }
                buttonReplace(spd_3_button,bmpToArgb(BUTTON_3_YELLOW,NULL,NULL));
                game_speed = 4;
                continue;
            }
        }
    }

    //remove buttons
    buttonDestroy(play_button);
    buttonDestroy(exit_button);
    buttonDestroy(spd_1_button);
    buttonDestroy(spd_2_button);
    buttonDestroy(spd_3_button);
    fontUnload(simfang);

    //return
    return ret;
}

bool pauseScreen () {
    //load font
    font* simfang = fontLoad("./assets/simfang.ttf");

    //initiate button
    struct button* play_button = buttonInit
        (bmpToArgb(PLAY_BUTTON_PATH,NULL,NULL),BUTTON_SIZE,BUTTON_SIZE,(lcd->w-BUTTON_SIZE)/2,lcd->h/2);
    struct button* exit_button = buttonInit
        (bmpToArgb(EXIT_BUTTON_PATH,NULL,NULL),BUTTON_SIZE,BUTTON_SIZE,lcd->w-BUTTON_SIZE,0);
    struct button* spd_1_button = buttonInit
        (bmpToArgb(game_speed==2?BUTTON_1_YELLOW:BUTTON_1_WHITE,NULL,NULL),BUTTON_SIZE,BUTTON_SIZE,(lcd->w-BUTTON_SIZE)/2-BUTTON_SIZE-10,lcd->h/2+BUTTON_SIZE+10);
    struct button* spd_2_button = buttonInit
        (bmpToArgb(game_speed==3?BUTTON_2_YELLOW:BUTTON_2_WHITE,NULL,NULL),BUTTON_SIZE,BUTTON_SIZE,(lcd->w-BUTTON_SIZE)/2,lcd->h/2+BUTTON_SIZE+10);
    struct button* spd_3_button = buttonInit
        (bmpToArgb(game_speed==4?BUTTON_3_YELLOW:BUTTON_3_WHITE,NULL,NULL),BUTTON_SIZE,BUTTON_SIZE,(lcd->w-BUTTON_SIZE)/2+BUTTON_SIZE+10,lcd->h/2+BUTTON_SIZE+10);


    //Print pause screen
    printText("已暂停",simfang,100,3,250,100);
    socrePrint();
    buttonAdd(play_button,lcd);
    buttonAdd(exit_button,lcd);
    buttonAdd(spd_1_button,lcd);
    buttonAdd(spd_2_button,lcd);
    buttonAdd(spd_3_button,lcd);

    //wait for start
    bool ret = false;
    while (1) {
        int action = ts->getAction(ts);
        if (action==TS_TAP) {
            if (isInButton(ts,play_button)) {
                ret = true;
                break;
            }
            if (isInButton(ts,exit_button)) {
                saveSync ();
                ret = false;
                break;
            }
            if (isInButton(ts,spd_1_button)) {
                if (game_speed==2) continue;
                switch (game_speed) {
                    case 3:
                        buttonReplace(spd_2_button,bmpToArgb(BUTTON_2_WHITE,NULL,NULL));
                        break;
                    case 4:
                        buttonReplace(spd_3_button,bmpToArgb(BUTTON_3_WHITE,NULL,NULL));
                        break;
                }
                buttonReplace(spd_1_button,bmpToArgb(BUTTON_1_YELLOW,NULL,NULL));
                game_speed = 2;
                continue;
            }
            if (isInButton(ts,spd_2_button)) {
                if (game_speed==3) continue;
                switch (game_speed) {
                    case 2:
                        buttonReplace(spd_1_button,bmpToArgb(BUTTON_1_WHITE,NULL,NULL));
                        break;
                    case 4:
                        buttonReplace(spd_3_button,bmpToArgb(BUTTON_3_WHITE,NULL,NULL));
                        break;
                }
                buttonReplace(spd_2_button,bmpToArgb(BUTTON_2_YELLOW,NULL,NULL));
                game_speed = 3;
                continue;
            }
            if (isInButton(ts,spd_3_button)) {
                if (game_speed==4) continue;
                switch (game_speed) {
                    case 2:
                        buttonReplace(spd_1_button,bmpToArgb(BUTTON_1_WHITE,NULL,NULL));
                        break;
                    case 3:
                        buttonReplace(spd_2_button,bmpToArgb(BUTTON_2_WHITE,NULL,NULL));
                        break;
                }
                buttonReplace(spd_3_button,bmpToArgb(BUTTON_3_YELLOW,NULL,NULL));
                game_speed = 4;
                continue;
            }
        }
    }

    //remove buttons
    buttonDestroy(play_button);
    buttonDestroy(exit_button);
    buttonDestroy(spd_1_button);
    buttonDestroy(spd_2_button);
    buttonDestroy(spd_3_button);
    fontUnload(simfang);

    //if all is good
    return ret;
}

int main () 
{
    //Initiate devices
    ts = TScreen_new(TS0);
    lcd = newScreen(LCD0);

    //set rand seed
    srand(time(NULL));

    //initiate score
    scoreInit();

    //start screen
    if (!startScreen()) goto exit;

    //clear board
    boardClear();


    //game screen here
    lcd->clear(lcd,ARGB_BLACK);
    snakeInit();

    //start the snake thread
    s_moving = 1;
    pthread_t snake_pid;
    pthread_create(&snake_pid,NULL,snakeTick,NULL);

    //main loop
    while (1) 
    {
        if (gg) {
            usleep(1000*100);
            continue;
        }

        switch (ts->getAction(ts)) 
        {
            case TS_UP:

                if (snake->heading==H_LEFT||snake->heading==H_RIGHT)
                    snake->heading = H_UP;
                break;
            case TS_DOWN:

                if (snake->heading==H_LEFT||snake->heading==H_RIGHT)
                    snake->heading = H_DOWN;
                break;
            case TS_LEFT:

                if (snake->heading==H_UP||snake->heading==H_DOWN)
                    snake->heading = H_LEFT;
                break;
            case TS_RIGHT:

                if (snake->heading==H_UP||snake->heading==H_DOWN)
                    snake->heading = H_RIGHT;
                break;
            case TS_TAP:
                if (gg) {
                    usleep(1000*100);
                    continue;
                }
                s_moving = 0;
                if (!pauseScreen()) goto exit;
                else
                {
                    boardRePaint();
                    s_moving = 1;
                }
        }

    }

    pthread_exit(&snake_pid);

exit:

    //clear screen
    lcd->clear(lcd,ARGB_BLACK);

    //close devices
    ts->remove(ts);
    lcd->remove(lcd);

    //ok
    return 0;
}
