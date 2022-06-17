/* See LICENSE file for copyright and license details. */
/* da minimalist toolkit
 * Copyright 2020,2021,2022 evv42.
 */
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT(x)
#define STBI_FAILURE_USERMSG
#define STBI_NO_HDR
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_PIC
#define STBI_NO_PNM
#include "images/stb_image.h"
#define OIOI_IMPLEMENTATION
#include "images/oioi.h"
#define MIMA_IMPLEMENTATION
#include "images/mima.h"
#include "dmtkgui.h"
#include "dmtk.h"
#include "fonts/unifont.h"
#include <string.h>
#include <stdio.h>

#define MTK_MAX_BUTTONS 64

ButtonArray* mtk_init_buttons(){
    ButtonArray* a = malloc(sizeof(ButtonArray));
    if(a == NULL){
        perror("malloc");
        exit(1);
    }
    a->bts = malloc(MTK_MAX_BUTTONS*sizeof(Button*));
    if(a->bts == NULL){
        perror("malloc");
        exit(1);
    }
    a->last_button = -1;
    return a;
}

void mtk_free_buttons(ButtonArray* a){
    for(int i=0; i<a->last_button; i++)free(a->bts[i]);
    free(a->bts);
    free(a);
}

Anchor mtk_put_image_buffer(DWindow* win, int h, int v, Image img){

	DDrawPixels(win, h, v, img.width, img.height, img.data);
	return (Anchor){.hxanchor = h + img.width+1, .hyanchor = v, .vxanchor = h, .vyanchor = v + img.height+1};
}

Image mtk_load_image(char* img){
    int sn;
	unsigned char mn;
	int x,y;
	char h = OIOI_HANDLED;
	unsigned char* d = oioi_read(img, &x, &y, 4);
    if(d == NULL){
		h = MIMA_HANDLED;
		d = mima_read(img, &x, &y, &mn, 4);
		if(d == NULL){
			h = STBI_HANDLED;
			d = stbi_load(img, &x, &y, &sn, 4);
		}
	}
	
	if(d == NULL){
		unsigned char* reason = malloc(strlen(stbi_failure_reason()));
		if(reason == NULL){
			perror("malloc");
			exit(1);
		}
		strcpy(reason,stbi_failure_reason());
        return (Image){.data=reason, .width=-1, .height=-1, .handler = ERROR};
    }
    
	return (Image){.data=(char*)d, .width=x, .height=y, .handler = h};
}

void mtk_free_image(Image img){
	switch(img.handler){
		case STBI_HANDLED:
			stbi_image_free(img.data);break;
		case OIOI_HANDLED:
		case MIMA_HANDLED:
			free(img.data);break;
		default:
			break;
	}
}

Anchor mtk_put_image(DWindow* win, int h, int v, char* file){
    Image img = mtk_load_image(file);
    
    Anchor a = mtk_put_image_buffer(win, h, v, img);
    mtk_free_image(img);
	return a;
}

int comp_index(const unsigned short* a, const unsigned short* b){
	return *a-*b;
}

int lookup_index(unsigned short c){
	unsigned short* p = bsearch(&c, dfont.index, dfont.chars, sizeof(unsigned short), (__compar_fn_t)comp_index);
    if(p == NULL)return lookup_index(' ');
	return p-dfont.index;
}

Anchor mtk_put_rectangle(DWindow* win, int h, int v, int sx, int sy, unsigned char r, unsigned char g, unsigned char b){
	DDrawRectangle(win, h, v, sx, sy, r, g, b);
	return (Anchor){.hxanchor = h + sx+1, .hyanchor = v, .vxanchor = h, .vyanchor = v + sy+1};
}

Anchor mtk_put_backbox(DWindow* win, int h, int v, int sx, int sy, int border){
	int db = 2 * border;
	DDrawRectangle(win, h-border, v-border, sx+db, sy+db, BGBTOP);
	Anchor a = mtk_put_rectangle(win, h, v, sx+border, sy+border, BGBBCK);
	DDrawRectangle(win, h, v, sx, sy, BGB);
	return a;
}

unsigned char mtk_font_width(){return dfont.width;}

unsigned char mtk_font_height(){return dfont.height;}

//Get Unicode codepoint of UTF-8 character
static unsigned int codepoint(char* str){
	if(*str & 128 && *str & 64){
		if(*str & 32){
			if(*str & 16)return (str[3] & 63) + ((str[2] & 63)<<6) + ((str[1] & 63)<<12) + ((str[0] & 7)<<18);
			return (str[2] & 63) + ((str[1] & 63)<<6) + ((str[0] & 15)<<12);
		}
		return (str[1] & 63) + ((str[0] & 31)<<6); 
	}
	return str[0];
}

//Get next UTF-8 character
static char* nextchar(char* str){
	if(*str & 128 && *str & 64){
		if(*str & 32){
			if(*str & 16)return str+4;
			return str+3;
		}
		return str+2; 
	}
	return str+1;
}

//Copy character bitmap to string bitmap
static char* copy_bitmap(char* bitmap, int offset, int bytes_per_line){
	for(int i = 0; i<((dfont.widths[offset]/8)*dfont.height); i++){
		*bitmap = dfont.bitmap[(offset*dfont.height*bytes_per_line)+ ((i%dfont.height)*bytes_per_line)+(i/dfont.height)];
		bitmap++;
	}
	return bitmap;
}

static char ispix(char* bitmap, int x, int y){
	return bitmap[((x/8)*dfont.height)+y] & (128 >> (x%8));
}

static char* render_str(char* bitmap, char* be, char r, char g, char b, int* x, int* y){
	char pix[4] = {r,g,b,255};
	char npix[4] = {0,0,0,0};
	
	*y = dfont.height;
	*x = ((be-bitmap)/dfont.height)*8;
	
	char* buf = malloc(*x * *y * 4);
	for(int py = 0; py < *y; py++){
		for(int px = 0; px < *x; px++){
			memcpy(buf + 4 *( (py * *x) + px), ispix(bitmap, px, py)? pix:npix,4);
		}
	}
	free(bitmap);
	return buf;
}

int mtk_str_width(char* string){
	int y = 0;
	while(*string != 0){
		y += dfont.widths[lookup_index(codepoint(string))];
		string = nextchar(string);
	}
	return y;
}

Anchor mtk_put_astring(DWindow* win, int h, int v, char* string, char r, char g, char b){
	if(*string == 0)return (Anchor){.hxanchor = h+1, .hyanchor = v, .vxanchor = h, .vyanchor = v+1};
	int bytes_per_line = (dfont.width/8);
	char* bitmap = malloc(strlen(string)*dfont.height*bytes_per_line);
	char* bend = bitmap;
	while(*string != 0){
		unsigned int unicode = codepoint(string);
		
		int offset = lookup_index(unicode);
		
		bend = copy_bitmap(bend,offset,bytes_per_line);
		
		string = nextchar(string);
	}
	int x,y;
	char* final = render_str(bitmap, bend, r, g, b, &x, &y);
	DDrawPixels(win, h, v, x , y, final);
	free(final);
	return (Anchor){.hxanchor = h + x+1, .hyanchor = v, .vxanchor = h, .vyanchor = v+y+1};
}

Anchor mtk_put_string(DWindow* win, int h, int v, char* string, char rb, char gb, char bb, char rf, char gf, char bf){
    Anchor a = mtk_put_rectangle(win, h, v, mtk_str_width(string), dfont.height, rb, gb, bb);
    mtk_put_astring(win, h, v, string, rf, gf, bf);
	return a;
}

void mtk_draw_button(DWindow* win, Button button){
    DDrawRectangle(win, button.ax, button.ay, button.bx-button.ax, button.by-button.ay, BTNTOP);
    DDrawRectangle(win, button.ax+1, button.ay+1, button.bx-button.ax-1, button.by-button.ay-1, BTNBCK);
    DDrawRectangle(win, button.ax+1, button.ay+1, button.bx-button.ax-2, button.by-button.ay-2, button.r, button.g, button.b);
    mtk_put_astring(win, button.ax+button.border, button.ay+button.border, button.text, 0, 0, 0);
}

void mtk_redraw_buttonarray(DWindow* win, ButtonArray* ba){
	for(int i = 0; i<=ba->last_button; i++)mtk_draw_button(win,*(ba->bts[i]));
}

int mtk_get_button(DWindow* win, ButtonArray* buttons, int x, int y){
    for(int i = 0; i<=buttons->last_button; i++){
        if(x>=buttons->bts[i]->ax && x<=buttons->bts[i]->bx && y>=buttons->bts[i]->ay && y<=buttons->bts[i]->by)return i;
    }
    return -1;
}

Button* mtk_add_coloured_button(DWindow* win, ButtonArray* buttons, int h, int v, char* str, int border, char r, char g, char b){
	//realloc?
	if((buttons->last_button+1)%MTK_MAX_BUTTONS == 0 && buttons->last_button > 0){
		buttons->bts = realloc(buttons->bts,(buttons->last_button+1+MTK_MAX_BUTTONS)*sizeof(Button*));
		if(buttons->bts == NULL){
			perror("realloc");
			exit(1);
		}
	}
    int slen = strlen(str);
    // Spaces + Border + Character size
    int sx = border*2 + mtk_str_width(str);
    int sy = border*2 + dfont.height;
    buttons->last_button += 1;
	buttons->bts[buttons->last_button] = malloc(sizeof(Button));
	if(buttons->bts[buttons->last_button] == NULL){
		perror("malloc");
		exit(1);
	}
    *(buttons->bts[buttons->last_button]) = (Button){.ax = h, .ay = v, .bx = h + sx, .by = v + sy, .border=border, .text = str, .hxanchor = h + sx + 1, .hyanchor = v, .vxanchor = h, .vyanchor = v + sy + 1, .r = r, .g = g, .b = b};
    mtk_draw_button(win, *(buttons->bts[buttons->last_button]));
    return buttons->bts[buttons->last_button];
}

Button* mtk_add_button(DWindow* win, ButtonArray* buttons, int h, int v, char* str, int border){
    return mtk_add_coloured_button(win, buttons, h, v, str, border, BTN);
}
