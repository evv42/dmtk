/* See LICENSE file for copyright and license details. */
/* dmtk renderer
 * Copyright 2020,2021,2022 evv42.
*/
# ifndef __DMTKGUI_H__
# define __DMTKGUI_H__

//Standard keys, based on X11

//Editing keys

#define SK_BackSpace                     0xff08
#define SK_Return                        0xff0d
#define SK_Insert                        0xff63
#define SK_Delete                        0xffff

//System keys

#define SK_Escape                        0xff1b
#define SK_Menu                          0xff67
#define SK_Super_L                       0xffeb  //Super or Command or Windows
#define SK_Super_R                       0xffec  //Super or Command or Windows
#define SK_Print                         0xff61
#define SK_Sys_Req                       0xff15
#define SK_Pause                         0xff13  //Pause

//Navigation keys

#define SK_Tab                           0xff09
#define SK_Home                          0xff50
#define SK_End                           0xff57
#define SK_Page_Up                       0xff55
#define SK_Page_Down                     0xff56
#define SK_Left                          0xff51
#define SK_Up                            0xff52
#define SK_Right                         0xff53
#define SK_Down                          0xff54

//Keypad

#define SK_Num_Lock                      0xff7f
#define SK_KP_Enter                      0xff8d
#define SK_KP_Home                       0xff95
#define SK_KP_Left                       0xff96
#define SK_KP_Up                         0xff97
#define SK_KP_Right                      0xff98
#define SK_KP_Down                       0xff99
#define SK_KP_Page_Up                    0xff9a
#define SK_KP_Next                       0xff9b
#define SK_KP_Page_Down                  0xff9b
#define SK_KP_End                        0xff9c
#define SK_KP_Insert                     0xff9e
#define SK_KP_Delete                     0xff9f
#define SK_KP_Equal                      0xffbd
#define SK_KP_Multiply                   0xffaa
#define SK_KP_Add                        0xffab
#define SK_KP_Separator                  0xffac  //Comma or dot
#define SK_KP_Subtract                   0xffad
#define SK_KP_Decimal                    0xffae
#define SK_KP_Divide                     0xffaf

#define SK_KP_0                          0xffb0
#define SK_KP_1                          0xffb1
#define SK_KP_2                          0xffb2
#define SK_KP_3                          0xffb3
#define SK_KP_4                          0xffb4
#define SK_KP_5                          0xffb5
#define SK_KP_6                          0xffb6
#define SK_KP_7                          0xffb7
#define SK_KP_8                          0xffb8
#define SK_KP_9                          0xffb9

//Function keys

#define SK_F1                            0xffbe
#define SK_F2                            0xffbf
#define SK_F3                            0xffc0
#define SK_F4                            0xffc1
#define SK_F5                            0xffc2
#define SK_F6                            0xffc3
#define SK_F7                            0xffc4
#define SK_F8                            0xffc5
#define SK_F9                            0xffc6
#define SK_F10                           0xffc7
#define SK_F11                           0xffc8
#define SK_F12                           0xffc9

//Modifiers

#define SK_Shift_L                       0xffe1
#define SK_Shift_R                       0xffe2
#define SK_Control_L                     0xffe3
#define SK_Control_R                     0xffe4
#define SK_Scroll_Lock                   0xff14
#define SK_Caps_Lock                     0xffe5
#define SK_Alt_L                         0xffe9
#define SK_Alt_R                         0xffea


#include <stdint.h>

//This will affect memory usage.
//A bigger buffer means faster draws of images.
#define MTK_IMAGE_CHUNK_SIZE 4*1024*4*256

#define PIXEL_RQ 0
#define RECTANGLE_RQ 1
#define WNAME_RQ 2
#define FLUSH_RQ 3
struct DrawRequest{
	char type;
	int h, v;
	int sx, sy;
	char* data;
	unsigned char r, g, b;
};
typedef struct DrawRequest DrawRequest;

#define KEYB_RQ 0
#define MOUSE_RQ 1
#define RESIZE_RQ 2
#define CLOSE_RQ 3
#define NO_RQ 4
struct GUIRequest{
	char type;
	uint32_t data;
	char utfkey[4];//Only for KEYB_RQ, 0-terminated character, including ASCII control.
	int x;
	int y;
};
typedef struct GUIRequest GUIRequest;

struct DWindow{
	int rx;
	int ry;
	char drawrq;
	DrawRequest drq;
	char guirq;
	GUIRequest grq;
	char alive;
};
typedef struct DWindow DWindow;

/* Creates a window. */
DWindow* DInit(int x, int y, char* name);

/* Creates a window that starts in the corner, without border. */
DWindow* DInitBorderless(int x, int y, char* name);

void DDrawPixels(DWindow* win, int h, int v, int sx, int sy, char* data);

void DDrawRectangle(DWindow* win, int h, int v, int sx, int sy, unsigned char r, unsigned char g, unsigned char b);

void DChangeName(DWindow* win, char* str);

void DFlush(DWindow* win);

GUIRequest DGetRequest(DWindow* win);

/* Deletes a window */
void DEndProcess(DWindow* win);
# endif
