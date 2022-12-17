#define DMTK_IMPLEMENTATION
#include "dmtk.h"
#include <stdlib.h>//rand

int main(){

    DWindow* win = DInit(1024,768,"Rectangles Speed Test");
	long unsigned int todraw = 420000;

    while(win->alive && todraw > 0){
		DGetRequest(win);
        DDrawRectangle(win, rand()%(1024/2), rand()%(768/2), rand()%1024, rand()%768, rand()%256, rand()%256, rand()%256);
        if(todraw%80 == 1)DFlush(win);
		todraw--;
    }
    
    DEndProcess(win);

    return 0;
}
