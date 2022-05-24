/* See LICENSE file for copyright and license details. */
/* dmtk standard input/output
 * Copyright 2022 evv42.
*/
#include "dmtkgui.h"
#include "dmtk.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MSG 0
#define STR 1
#define INT 2

#define OK "OK"
#define BMARGIN 6
#define BSHFT BMARGIN*3
#define STRSIZE 64

static void* mtksio(char* title, char* message, char type){
	ButtonArray* b;
	Anchor a;
	char* input = malloc(STRSIZE+1);//You should not make people type more than that.
	if(input == NULL)return input;
	memset(input,0,STRSIZE+1);
	char inpos = 0;
	DWindow* subw = DInit(150+mtk_str_width(message),5*mtk_font_height(),title);
	b = mtk_init_buttons();
	mtk_add_button(subw,b,subw->rx-BSHFT-mtk_str_width(OK),subw->ry-mtk_font_height()-BSHFT,OK,BMARGIN);
	while(subw->alive){
		GUIRequest grq = DGetRequest(subw);
		switch(grq.type){
			case RESIZE_RQ:
				mtk_put_rectangle(subw, 0, 0, subw->rx, subw->ry, BG);
				a = mtk_put_astring(subw, BMARGIN, BMARGIN, message, FG);
				if(type != MSG)mtk_put_string(subw, a.vxanchor, a.vyanchor, input, WHT, FG);
				mtk_redraw_buttonarray(subw,b);
				DFlush(subw);
				break;
			case KEYB_RQ:
				mtk_put_rectangle(subw, a.vxanchor, a.vyanchor, mtk_str_width(input),mtk_font_height(), BG);
				if(grq.data == SK_Return || grq.data == SK_KP_Enter)DEndProcess(subw);
				if(grq.utfkey[0] == 0x08 && inpos > 0){
					input[--inpos] = 0;
					while(input[inpos-1] & 0x80 && (!(input[inpos-1] & 0xC0)) && inpos > 0)input[--inpos] = 0;
					if(input[inpos-1] > 0xC0 && inpos > 0)input[--inpos] = 0;
				}else if( (inpos + strlen(grq.utfkey)) < STRSIZE ){
					strcpy(input+inpos,grq.utfkey);
					inpos += strlen(grq.utfkey);
				}
				if(type != MSG)mtk_put_string(subw, a.vxanchor, a.vyanchor, input, WHT, FG);
				DFlush(subw);
				break;
			case MOUSE_RQ:
				if(mtk_get_button(subw,b,grq.x,grq.y) != -1)DEndProcess(subw);
				break;
			default:
				break;
		}
		usleep(15000);
	}
	
	if(type == STR)return input;
	if(type == INT){
		int* i = malloc(sizeof(int));
		if(i == NULL)return i;
		*i = atoi(input);
		free(input);
		return i;
	}
	free(input);
	return NULL;
}

void mtksiomsg(char* title, char* message){
	mtksio(title, message, MSG);
}

char* mtksiostr(char* title, char* message){
	return mtksio(title, message, STR);
}

int mtksioint(char* title, char* message){
	int* i = mtksio(title, message, INT);
	int j = *i;
	free(i);
	return j;
}
