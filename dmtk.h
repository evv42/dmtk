/* See LICENSE file for copyright and license details. */
/* da minimalist toolkit
 * Copyright 2020,2021,2022 evv42.
 */
# ifndef __DMTK_H__
# define __DMTK_H__

//Default colors: You may use them.
#define BG 222,223,224        //Default window background
#define FG 0,0,0              //Default foreground (text)
#define BORDER 99,99,99       //Default window border
#define SEP 189,189,189       //Default window separation
#define BTN 236,237,238       //Default button
#define BTNTOP 245,245,245    //Default button top border
#define BTNBCK 180,180,180    //Default button back border
#define BGB 192,192,192       //Default Z-1 background
#define BGBTOP 100,100,100    //Default Z-1 top border
#define BGBBCK 245,245,245    //Default Z-1 back border

//Aliases
#define BLK 0,0,0
#define WHT 255,255,255
#define RED 255,0,0
#define CYN 0,255,255
#define PUR 255,0,255
#define GRN 0,255,0
#define BLU 0,0,255
#define YEL 255,255,0

struct Anchor{
	int hxanchor;//Right
	int hyanchor;
	int vxanchor;//Down
	int vyanchor;
};
typedef struct Anchor Anchor;

// Image related functions

#define OIOI_HANDLED 0
#define STBI_HANDLED 1
#define MIMA_HANDLED 2
#define ERROR 3
struct Image{
	char* data;
	int width;
	int height;
	char handler;
};
typedef struct Image Image;

Anchor mtk_put_image_buffer(DWindow* win, int h, int v, Image img);

Image mtk_load_image(char* img);

void mtk_free_image(Image img);

//Put image from file using functions above
Anchor mtk_put_image(DWindow* win, int h, int v, char* file);

/* Drawing funtions */

Anchor mtk_put_rectangle(DWindow* win, int h, int v, int sx, int sy, unsigned char r, unsigned char g, unsigned char b);

//Put Z-1 box
Anchor mtk_put_backbox(DWindow* win, int h, int v, int sx, int sy, int border);


/* Text rendering functions
 * For simplicity sake, MTK does use a single monospaced font.
 */

//Return character max width
unsigned char mtk_font_width();

//Return width of string
int mtk_str_width(char* string);

//Return character height
unsigned char mtk_font_height();

//Put a string over the existing background
Anchor mtk_put_astring(DWindow* win, int h, int v, char* string, char r, char g, char b);

//Put a backgrounded string
Anchor mtk_put_string(DWindow* win, int h, int v, char* string, char rb, char gb, char bb, char rf, char gf, char bf);

/* Button functions */

struct Button{
    int ax;
    int ay;
    int bx;
    int by;
    int border;
    char* text;
	int hxanchor;//Anchor for next button (right)
	int hyanchor;
	int vxanchor;//Anchor for next button (down)
	int vyanchor;
	unsigned char r;
	unsigned char g;
	unsigned char b;
};
typedef struct Button Button;

struct ButtonArray{
    Button** bts;
    int last_button;
};
typedef struct ButtonArray ButtonArray;

ButtonArray* mtk_init_buttons();

void mtk_free_buttons(ButtonArray* a);

void mtk_redraw_buttonarray(DWindow* win, ButtonArray* ba);

Button* mtk_add_button(DWindow* win, ButtonArray* buttons, int h, int v, char* str, int border);

Button* mtk_add_coloured_button(DWindow* win, ButtonArray* buttons, int h, int v, char* str, int border, char r, char g, char b);

int mtk_get_button(DWindow* win, ButtonArray* buttons, int x, int y);
# endif
