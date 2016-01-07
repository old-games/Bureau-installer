//Dimouse's GUI library
#ifndef DGUI_H
#define DGUI_H

#ifndef ALLEGRO_STATICLINK
#define ALLEGRO_STATICLINK
#endif

#include "allegro.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <fstream.h>
#include <iostream.h>
//#include "dwgra.h"

int load_font(char *,BITMAP *); //load font (BITMAP 256x128 pixels, each char is 8x16 pixels) from file
unsigned char to_rus(unsigned char c); //convert char from WIN to DOS russian charset (if font is DOS should be used)

class dwindow{
	public:
	int x; //coord of top in x
	int y; //coord of top in y
	int w; //width
	int h; //height

	BITMAP *back; //background of this window
	BITMAP *tmp; //temp bitmap for drawing
	BITMAP *fnt; //font used for text on this window

	dwindow *parent; //parent window which it is drawn on
	dwindow *s[10]; //siblings
	int s_type[10];
	int n_siblings; //number of siblings
	int option; //by default option=0 if window is not pressed by left mouse button and 1 otherwise

	dwindow();
	dwindow(int,int,int,int,BITMAP*);
	dwindow(int,int,int,int,BITMAP*,BITMAP*);
	~dwindow(){destroy_bitmap(back);destroy_bitmap(tmp);} //default destructor
	void save_back(){blit(parent->back,back,x-parent->x,y-parent->y,0,0,w,h);}
	void restore_back(){blit(back,parent->back,0,0,x-parent->x,y-parent->y,w,h);}
	void clear_back(){clear_to_color(back, makecol(255, 0, 255));}
	void set_back(BITMAP* b){back=b;}
	void setFnt(BITMAP *f){fnt=f;}
	void setBack(BITMAP *b){back=b;}

	void draw_char(int x, int y, char chr); //draw char on BITMAP
	void outtextxy(int x,int y, char* string);
	void outtextxy2(int x,int y, char* string);

	int create_sibling(int,int,int,int,BITMAP *);
	int create_ani_sibling(int,int,int,int,BITMAP**,int);
	int create_inp_sibling(int,int,int,int,int,int);
	int create_txt_sibling(int,int,int,int,char*);
	int delete_sibling(int);

	void show(BITMAP *where);
	void show2(BITMAP *where);
	void show_all(BITMAP *where); //recursive drawing of all siblings

	int is_selected(); //is it pressed by mouse?
	int get_option(){return option;}
	void set_option(int o){option=o;}
	int is_selected_sib(int); //is n-th sibling is pressed by mouse?
};

class input_dwindow:public dwindow{
//window with input
	public:
	int num_min; //min chars to input
	int num_max; //max chars to input
	int status; //is it empty or alreay used to write smth
	char buffer[100];
	int current;

	input_dwindow(int,int,int,int,BITMAP *,int,int);
	int input();
	void set_min(int n){num_min=n;}
	void set_max(int n){num_max=n;}
	void set_status(int ss){status=ss;}
	int get_status(){return status;}
	char * get_buffer(){return buffer;}
	void set_buffer(char *b){strcpy(buffer,b); outtextxy2(2, -4, buffer); current=strlen(buffer);}
	void set_border(int col){if(col==0) rect(tmp,0,0,w-1,h-1,makecol(0,0,255));if(col==1) rect(tmp,0,0,w-1,h-1,makecol(255,0,0));}

};

class ani_dwindow:public dwindow{
//animated window
	public:
	BITMAP *a[60]; //maximum frames=60
	int n_frames;
	int current;

	ani_dwindow(int,int,int,int,BITMAP*,BITMAP **,int);
	void showAni(BITMAP *where){masked_blit(a[current],where,0,0,x,y,w,h);current++;if(current==60)current=0;}
};

class txt_dwindow:public dwindow{
//animated window
	public:
	char name[20];
	int n_text; //number of text messages
	char t[20][40]; //up to 20 text messages, up to 40 chars each (width of 320x200 window)
	int t_x[20],t_y[20]; //coords of text output
	int showtime; //aka frame
	int showopt; //to show different effects

	txt_dwindow(int,int,int,int,BITMAP*,char *);
	int load_text(); //load text from messages file
	void print_text(); //print text on back
	void showTxt(BITMAP *where){showtext(showtime,where);if(showtime!=50)showtime++;}
	void showTxt_all(BITMAP *where); //recursive drawing of all siblings
	void set_text(int i,char* text){if(i<n_text)strcpy(t[i],text);clear_back();print_text();}
	void set_time(int t){showtime=t;}
	void showtext(int frame,BITMAP *where); //this is various effects to show text on screen
	void set_showopt(int o){showopt=o;}
};
#endif
