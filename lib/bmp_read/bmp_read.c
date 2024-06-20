#include "bmp_read.h"

unsigned int* bmpToArgb(char* path, int* width, int* height) {
    //open the bmp file
    FILE* bmp_file = fopen(path,"r");
    if(bmp_file==NULL){
        perror("invalid file path");
        goto open_error;
    }

    //read the bmp header
    struct BitFileHeader bmp_head;
    if(fread(&bmp_head,1,sizeof(struct BitFileHeader),bmp_file)==-1) {
        perror("invalid file");
        goto general_error;
    }

    //put data into width and height pointer, if said pointer exists
    if(width!=NULL) *width = bmp_head.biWidth;
    if(height!=NULL) *height = bmp_head.biHeight;

    //make space for the data
    unsigned int* data = (int*)malloc(bmp_head.biHeight*bmp_head.biWidth*4);
    if(data==NULL) {
        perror("failed to make space for bmp data");
        goto general_error;
    }

    //loop convert and put data
    unsigned int* p = data;
    unsigned char mem_buffer[3];
    for(int i=0;i<bmp_head.biWidth*bmp_head.biHeight;i++) {
        fread(mem_buffer,1,3,bmp_file);
        *p++ = 0xff<<24|mem_buffer[2]<<16|mem_buffer[1]<<8|mem_buffer[0];
    }

    //if all is good
    fclose(bmp_file);
    return data;

general_error:
    fclose(bmp_file);
open_error:
    return NULL;
} 