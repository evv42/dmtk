/* See LICENSE file for copyright and license details. */
/* dmtk renderer
 * X11/Xlib Version
 * Copyright 2020,2021,2022 evv42.
*/
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/Xlocale.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include "dmtkgui.h"

#define TYPE_NORMAL 0
#define TYPE_DESKAPP 1 //Makes the window spawn at the top-left of the screen, without border.

struct XlibWin{
	Display* dis;
	int screen;
	Window xw;
	GC gc;
	Atom wm_delete_window;
};
typedef struct XlibWin XlibWin;

static void DGUIProcess(DWindow* win, char type, char* name);

void DEndProcess(DWindow* win){
	win->guirq = 1;
	win->grq.type = CLOSE_RQ;
}

static DWindow* DInitGen(int x, int y, char type, char* name){
	int protection = PROT_READ | PROT_WRITE;
	int visibility = MAP_SHARED | MAP_ANONYMOUS;
    DWindow* w = mmap(NULL, sizeof(DWindow), protection, visibility, -1, 0);
    if(w == MAP_FAILED){
        perror("mmap");
        exit(1);
    }
	w->rx = x;
	w->ry = y;
	w->guirq = 0;
	w->drawrq = 0;
	w->alive = 1;
	w->drq.data = mmap(NULL, MTK_IMAGE_CHUNK_SIZE, protection, visibility, -1, 0);
	if(w->drq.data == MAP_FAILED){
        perror("mmap");
        exit(1);
    }
    
	int pid = fork();
    
	if (pid == 0) {
		DGUIProcess(w,type,name);
	}
	return w;
}

DWindow* DInit(int x, int y, char* name){
	return DInitGen(x, y, TYPE_NORMAL, name);
}

DWindow* DInitBorderless(int x, int y, char* name){
	return DInitGen(x, y, TYPE_DESKAPP, name);
}

static XlibWin init_x(int rx, int ry, int type, char* name){    
	setlocale(LC_ALL, "");
	XlibWin xlw = {0};
	unsigned long black,white;

	/* use the information from the environment variable DISPLAY
	   to create the X connection:
	*/
    char *dn = getenv("DISPLAY");
	xlw.dis=XOpenDisplay(dn);
   	xlw.screen=DefaultScreen(xlw.dis);
	black=BlackPixel(xlw.dis, xlw.screen),	/* get color black */
	white=WhitePixel(xlw.dis, xlw.screen);  /* get color white */

   	xlw.xw=XCreateSimpleWindow(xlw.dis,DefaultRootWindow(xlw.dis),0,0,rx, ry, 5, black, white);

	//select allowed inputs
	XSelectInput(xlw.dis, xlw.xw, ExposureMask|ButtonPressMask|KeyPressMask);
	
	//create gc
	xlw.gc=XCreateGC(xlw.dis, xlw.xw, 0,0);

	//set def col
	XSetBackground(xlw.dis,xlw.gc,white);
	XSetForeground(xlw.dis,xlw.gc,black);

	//clear win
	XClearWindow(xlw.dis, xlw.xw);

    if(type == TYPE_DESKAPP){
        //Set window type
        Atom window_type = XInternAtom(xlw.dis, "_NET_WM_WINDOW_TYPE", False);
        long value = XInternAtom(xlw.dis, "_NET_WM_WINDOW_TYPE_SPLASH", False);
        XChangeProperty(xlw.dis, xlw.xw, window_type, XA_ATOM, 32, PropModeReplace, (unsigned char *) &value, 1);

        //Set window state
        window_type = XInternAtom(xlw.dis, "_NET_WM_STATE", False);
        value = XInternAtom(xlw.dis, "_NET_WM_WINDOW_STATE_ABOVE", False);
        XChangeProperty(xlw.dis, xlw.xw, window_type, XA_ATOM, 32, PropModeReplace, (unsigned char *) &value, 1);
    }

    //set win name (mantadory)
	XStoreName(xlw.dis,xlw.xw,name);

	XMapRaised(xlw.dis, xlw.xw);
	xlw.wm_delete_window = XInternAtom(xlw.dis, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(xlw.dis, xlw.xw, &xlw.wm_delete_window, 1);
	XSync(xlw.dis, False);
	
	return xlw;
}

static void close_x(XlibWin xlw) {
	XFreeGC(xlw.dis, xlw.gc);
	XDestroyWindow(xlw.dis,xlw.xw);
	XCloseDisplay(xlw.dis);
	exit(0);
}

static void cut_image(char* image, int xo, int yo, int xf, int yf){
	int rest = (xo*yo*4) - (xf*4);
	while(yf>0){
		memmove(image+(xf*4),image+(xo*4),rest);
		rest -= (xf*4);
		yf -= 1;
		image += (xf*4);
	}
}

static void DAcceptDrawRequest(XlibWin xlw, DWindow* win){
	//Check if the user has been silly, and have not waited for the redraw to RESIZE_RQ
	//If the request is a name change, we can however let it pass
	if(win->guirq == 1 && win->grq.type == RESIZE_RQ && win->drq.type != WNAME_RQ){
		//printf("DRAW INTERRUPTED !\n");
		win->drawrq = 0;
		return;
	}
	
	//WANING: This does not make a copy of the data buffer.
	//Set win->drawrq to 0 only when the data buffer has been used.
	DrawRequest drq = win->drq;
	
	switch(drq.type){
		case PIXEL_RQ:
			//Check if the dev has been silly, and have tried drawing out of bounds
			if(drq.h+drq.sx > win->rx || drq.v+drq.sy > win->ry){
				if(drq.h > win->rx || drq.v > win->ry){
					printf("Out of bounds !\n");
					fflush(stdout);
					//win->drawrq = 0;
					return;
				}
				int xf = drq.sx < win->rx-drq.h ? drq.sx : win->rx-drq.h;
				int yf = drq.sy < win->ry-drq.v ? drq.sy : win->ry-drq.v;
				cut_image(drq.data, drq.sx, drq.sy, xf, yf);
				drq.sx = xf;
				drq.sy = yf;
			}
			char* drawbuf = malloc(drq.sx*drq.sy*4);
			memcpy(drawbuf,drq.data,drq.sx*drq.sy*4);
			win->drawrq = 0;
			XImage* image = XCreateImage(xlw.dis, DefaultVisual(xlw.dis, DefaultScreen(xlw.dis)), 24, ZPixmap, 0, drawbuf, drq.sx, drq.sy, 32, 0);
			//printf("%d,%d,%d,%d,%d,%d\n", win->rx, win->ry, drq.h, drq.v, drq.sx, drq.sy);
			//fflush(stdout);
			//Get window image, so we can properly do alpha channel.
			XImage* scr = XGetImage(xlw.dis,xlw.xw, drq.h, drq.v, drq.sx, drq.sy, 0xFFFFFFFF, ZPixmap);
			
			//This part does RGB to BGR conversion, and alpha channel calc.
			char* srcNdest=image->data; char* old=scr->data; char srcRED;
			for(int i=0; i<drq.sx*drq.sy; i++){
				srcRED = srcNdest[0];//Remove if you hate red
				srcNdest[0] = ((srcNdest[2] * srcNdest[3] ) + (old[0] * (0xFF-srcNdest[3]) ))/0xFF;
				srcNdest[1] = ((srcNdest[1] * srcNdest[3] ) + (old[1] * (0xFF-srcNdest[3]) ))/0xFF;
				srcNdest[2] = ((srcRED      * srcNdest[3] ) + (old[2] * (0xFF-srcNdest[3]) ))/0xFF;
				srcNdest[3] = 0xFF;
				srcNdest+=4;
				old+=4;
			}
			
			XPutImage(xlw.dis, xlw.xw, xlw.gc, image, 0, 0, drq.h, drq.v, drq.sx, drq.sy);
			XDestroyImage(image);
			XDestroyImage(scr);
			return;
		case RECTANGLE_RQ:
			win->drawrq = 0;
			if(drq.sx < 1 || drq.sy < 1)return;
			XSetForeground(xlw.dis,xlw.gc,(drq.r<<16) + (drq.g<<8) + drq.b);
			XDrawRectangle(xlw.dis, xlw.xw, xlw.gc, drq.h, drq.v, drq.sx-1, drq.sy-1);
			XFillRectangle(xlw.dis, xlw.xw, xlw.gc, drq.h, drq.v, drq.sx-1, drq.sy-1);
			return;
		case WNAME_RQ:
			XStoreName(xlw.dis,xlw.xw,drq.data);
			win->drawrq = 0;
			return;
		case FLUSH_RQ:
			win->drawrq = 0;
			XSync(xlw.dis,False);
			return;
		default:
			return;
	}
}

static void DEmitResizeRequest(DWindow* win, int x, int y){
	if(x>=0)win->rx = x;
	if(y>=0)win->ry = y;
	win->guirq = 1;
	win->grq.type = RESIZE_RQ;
}

static void DGUIProcess(DWindow* win, char type, char* name){
	XlibWin xlw = init_x(win->rx, win->ry, type, name);usleep(50000);
	XEvent event;//event buffer
	XWindowAttributes xa = {0};
	int nx,ny;
	while(1) {
		usleep(5000);
        if(XPending(xlw.dis) != 0){
			XNextEvent(xlw.dis, &event);
			KeySym key;
			char text[4];
			int ts;
			switch(event.type){
				case Expose:
					nx=-1,ny=-1;
					XGetWindowAttributes(xlw.dis, xlw.xw, &xa);
					if(xa.width != win->rx || xa.height != win->ry){nx=xa.width;ny=xa.height;}
					if(event.xexpose.count == 0)DEmitResizeRequest(win, nx, ny);
					break;
				case KeyPress:
					if(!win->guirq){
						ts = XLookupString(&event.xkey,text,4,&key,0);
						win->grq.type = KEYB_RQ;
						win->grq.data = (uint32_t)key;
						memset(win->grq.utfkey, 0, 4);
						memcpy(win->grq.utfkey, text, ts);
						win->guirq = 1;
					}
					break;
				case ButtonPress:
					if(!win->guirq){
						win->grq.type = MOUSE_RQ;
						win->grq.data = event.xbutton.button;
						win->grq.x = event.xbutton.x;
						win->grq.y = event.xbutton.y;
						win->guirq = 1;
					}
					break;
				case ClientMessage:
					if ((Atom)event.xclient.data.l[0] == xlw.wm_delete_window) {
						win->alive = 0;
						win->guirq = 0;
						win->drawrq = 0;
						close_x(xlw);
					}
					break;
			}
		}
		if(win->guirq && win->grq.type == CLOSE_RQ){
			win->alive = 0;
			win->guirq = 0;
			win->drawrq = 0;
			close_x(xlw);
		}
		if(win->drawrq){
			while(win->drq.type != FLUSH_RQ){
				if(win->drawrq)DAcceptDrawRequest(xlw, win);
				usleep(1);
			}
			DAcceptDrawRequest(xlw, win);
		}
	}
}

//Main process functions

void DDrawPixels(DWindow* win, int h, int v, int sx, int sy, char* data){
	if(sx*sy*4 < MTK_IMAGE_CHUNK_SIZE){
		win->drq.type = PIXEL_RQ;
		win->drq.h = h;
		win->drq.v = v;
		win->drq.sx = sx;
		win->drq.sy = sy;
		memcpy(win->drq.data,data,sx*sy*4);
		win->drawrq = 1;
		while(win->drawrq);
	}else{
		//how many lines can we store in the buffer ?
		int lines = MTK_IMAGE_CHUNK_SIZE / (sx*4);
		unsigned int vshift = 0; //shift for data
		//while there is lines to draw
		while(sy > 0){
			win->drq.type = PIXEL_RQ;
			win->drq.h = h;
			win->drq.v = v;
			win->drq.sx = sx;
			win->drq.sy = sy < lines ? sy : lines;
			memcpy(win->drq.data,data+vshift,win->drq.sx*win->drq.sy*4);
			//printf("%d,%d %dx%d %d\n",win->drq.h,win->drq.v,win->drq.sx,win->drq.sy,vshift);
			//fflush(stdout);
			win->drawrq = 1;
			while(win->drawrq);
			v += win->drq.sy;
			vshift += win->drq.sy*sx*4;
			sy -= win->drq.sy;
		}
	}
}

void DDrawRectangle(DWindow* win, int h, int v, int sx, int sy, unsigned char r, unsigned char g, unsigned char b){
	win->drq.type = RECTANGLE_RQ;
	win->drq.h = h;
	win->drq.v = v;
	win->drq.sx = sx;
	win->drq.sy = sy;
	win->drq.r = r;
	win->drq.g = g;
	win->drq.b = b;
	win->drawrq = 1;
	while(win->drawrq);
}

void DChangeName(DWindow* win, char* str){
	win->drq.type = WNAME_RQ;
	strcpy(win->drq.data,str);
	win->drawrq = 1;
	while(win->drawrq);
}

void DFlush(DWindow* win){
	win->drq.type = FLUSH_RQ;
	win->drawrq = 1;
	while(win->drawrq);
}

GUIRequest DGetRequest(DWindow* win){
	GUIRequest grq = {0};
	if(win->guirq){
		grq.type = win->grq.type;
		grq.data = win->grq.data;
		memcpy(grq.utfkey, win->grq.utfkey,4);
		grq.x = win->grq.x;
		grq.y = win->grq.y;
		win->guirq = 0;
	}else{
		grq.type = NO_RQ;
	}
	return grq;
}
