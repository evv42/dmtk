#include <time.h>
#include <stdio.h>
#include <unistd.h>
#define DMTK_IMPLEMENTATION
#include "dmtk.h"

int main(){
    time_t rawtime;
    struct tm* timeinfo;
    char* str_time;
	
	//Get a first string to calculate window size
	time(&rawtime);
	timeinfo = localtime(&rawtime);
    DWindow* win = DInit(mtk_str_width(asctime(timeinfo))+4,mtk_font_height()+4,"A Clock");

    while(win->alive){
		DGetRequest(win);
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        str_time = asctime(timeinfo);
        mtk_put_string(win, 2, 2, str_time,BG,BLK);
        DFlush(win);
        sleep(1);
    }

    return 0;
}
