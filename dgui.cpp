//input to the input window
#include "dgui.h"
#include <math.h>

//int stretch=3;

int load_font(char *filename,BITMAP *font){
BITMAP *tmpfnt;
PALETTE pal;
tmpfnt=load_bmp(filename,pal);
blit(tmpfnt,font,0,0,0,0,256,128);
return 1;
}

unsigned char to_rus(unsigned char c){
if(c>=224&&c<=239) return c-64;
if(c>=240) return c-16;
if(c>=192&&c<=223) return c-64;

return c;
}

//=======================================================DWINDOW
dwindow::dwindow(){
x=0;
y=0;
w=320;
h=200;
back=screen;
tmp=screen;
fnt=NULL;
parent=NULL;
for(int i=0;i<10;i++){s[i]=NULL;s_type[i]=0;}
n_siblings=0;
option=0;
}

dwindow::dwindow(int ix,int iy,int iw,int ih,BITMAP *f){
//constuctor which define members of the class
x=ix;
y=iy;
w=iw;
h=ih;
back=create_bitmap(iw,ih);
tmp=create_bitmap(iw,ih);
clear_to_color(tmp, makecol(255, 0, 255));
fnt=f;
parent=NULL;
for(int i=0;i<10;i++){s[i]=NULL;s_type[i]=0;}
n_siblings=0;
option=0;
}

dwindow::dwindow(int ix,int iy,int iw,int ih,BITMAP *f,BITMAP* b){
//constuctor which define members of the class with background
x=ix;
y=iy;
w=iw;
h=ih;
//back=create_bitmap(iw,ih);
tmp=create_bitmap(iw,ih);
clear_to_color(tmp, makecol(255, 0, 255));
fnt=f;
back=b;
parent=NULL;
for(int i=0;i<10;i++){s[i]=NULL;s_type[i]=0;}
n_siblings=0;
option=0;
}

void dwindow::draw_char(int x, int y, char chr){
//draw char on window at coord x,y
unsigned char p=to_rus(chr);
masked_blit(fnt,back,(p%32)*8,(p/32)*16,x,y,8,16);
}

void dwindow::outtextxy(int x,int y, char* string){
//centered text
int len=strlen(string)-1;
if(len>40)len=40;
int x0 = 160 - len*4; //40 chars width of window

for(int i=0;i<len;i++){
	draw_char(x+i*8+x0,y,string[i]);
}
}

void dwindow::outtextxy2(int x,int y, char* string){
//not centered text, with moving to next line
int len=strlen(string);
const int width=320; //width of screen

for(int i=0;i<len;i++){
draw_char((x+i*8)%width,y+(x+i*8)/width,string[i]);
}
}

int dwindow::create_sibling(int ix,int iy,int iw,int ih,BITMAP* b){
	if(n_siblings<10){
		s[n_siblings] = new dwindow(ix,iy,iw,ih,fnt,b);
		s[n_siblings]->parent=this;
		s_type[n_siblings]=0;
		n_siblings++;
		return 1;
	}
	else return 0; //number of siblings exceeded limit
}

int dwindow::create_ani_sibling(int ix,int iy,int iw,int ih,BITMAP *aa[],int n){
	if(n_siblings<10){
		s[n_siblings] = new ani_dwindow(ix,iy,iw,ih,fnt,aa,n);
		s[n_siblings]->parent=this;
		s_type[n_siblings]=1;
		n_siblings++;
		return 1;
	}
	else return 0; //number of siblings exceeded limit
}

int dwindow::create_txt_sibling(int ix,int iy,int iw,int ih,char *name){
	if(n_siblings<10){
		s[n_siblings] = new txt_dwindow(ix,iy,iw,ih,fnt,name);
		s[n_siblings]->parent=this;
		s_type[n_siblings]=2;
		n_siblings++;
		return 1;
	}
	else return 0; //number of siblings exceeded limit
}

int dwindow::create_inp_sibling(int ix,int iy,int iw,int ih, int min, int max){
	if(n_siblings<10){
		s[n_siblings] = new input_dwindow(ix,iy,iw,ih,fnt,min,max);
		s[n_siblings]->parent=this;
		s_type[n_siblings]=3;
		n_siblings++;
		return 1;
	}
	else return 0; //number of siblings exceeded limit
}

int dwindow::delete_sibling(int n){
if(n>n_siblings||n<1)return -1; //ERROR: fool's proof
//for(int i=0;i<s[n]->n_siblings;i++) s[n]->delete_sibling(i);
for(int i=n;i<n_siblings-1;i++) s[i]=s[i+1];
n_siblings--;

return 1;
}

void dwindow::show(BITMAP *where){
	masked_blit(tmp,where,0,0,x,y,w,h);
	//else masked_blit(back,parent->tmp,0,0,x,y,w,h);
}

void dwindow::show2(BITMAP *where){
//	masked_blit(tmp,where,0,0,x,y,w,h);
	//else masked_blit(back,parent->tmp,0,0,x,y,w,h);
}

void dwindow::show_all(BITMAP *where){
if(n_siblings!=0)blit(back,tmp,0,0,0,0,w,h); //save back to tmp
else tmp=back;
//show(where);
int stat=0;
for(int i=0;i<n_siblings;i++){
	if(s_type[i]==0)s[i]->show_all(tmp);
	if(s_type[i]==1)((ani_dwindow*)s[i])->showAni(tmp);
	if(s_type[i]==2)((txt_dwindow*)s[i])->showTxt_all(tmp);
	if(s_type[i]==3){
		//if(s[i]->is_selected()&&stat==0){
			stat=1; //please do not use several inputs from different parents simultaniously
			((input_dwindow*)s[i])->set_status(1); //status=1 means this input box is currently in use
			((input_dwindow*)s[i])->set_border(1);
			//((input_dwindow*)s[i])->input();
		//}
		s[i]->show_all(tmp);
	}
}
show(where);
}

int dwindow::is_selected(){
//TODO!
if(mouse_b & 1){
//	scare_mouse();
//	int option=0;
	double scale_x=double(SCREEN_W)/double(parent->w);
	double scale_y=double(SCREEN_H)/double(parent->h);

	if(mouse_x > x*scale_x && mouse_x < (x+w)*scale_x && mouse_y > y*scale_y && mouse_y < (y+h)*scale_y){
		//blit(chkbox2,back2,0,0,0,100,23,19);
		//blit2(back2,screen,0,0,320,200,0,0,640,400);
		if(option==0)option=1;
		else option=0;
	}
}
return option;
}

int dwindow::is_selected_sib(int n){
	if(n>n_siblings||n<0) return -1; //ERROR: fool's proof
	return s[n]->is_selected();
}

//========================================================INPUT DWINDOW
input_dwindow::input_dwindow(int ix,int iy,int iw,int ih,BITMAP *f,int min,int max){
//default constructor
x=ix;
y=iy;
w=iw;
h=ih;
back=create_bitmap(iw,ih);
tmp=create_bitmap(iw,ih);
clear_to_color(back, makecol(255, 0, 255));
rect(back,0,0,w-1,h-1,makecol(0,0,255));

fnt=f;
parent=NULL;
int i;
for(i=0;i<10;i++){s[i]=NULL;s_type[i]=0;}
n_siblings=0;

num_min=min;
num_max=max;
option=0;
status=0;
current=0;

for(i=0;i<num_max;i++)
	buffer[i]=0;

buffer[num_max]='\0';
}

//==================================INPUT
int input_dwindow::input(){
//actual input in input_dwindow
status=1; //box is in use
if(keypressed()){
	int key = readkey();
	if (key>>8 == KEY_BACKSPACE){ //delete char
		if(current>0){
			buffer[current-1]=' ';
			current--;
		}
	}
	else if (key>>8 == KEY_ENTER){ //finish with input
		if(current>=num_min-1) status=2;//return_value=1;
	}
	else if (key>>8 == KEY_ESC){
		for(int i=0;i<num_max;i++) //clear buffer
		buffer[i]=0;
		buffer[num_max]='\0';
		current=0;
		status=0; //box is not in use
	}
	else{
		if((key & 0xff) >= ' '){//if(isalpha(key & 0xff) || (key & 0xff) == '\\' || (key & 0xff) == ':'){
			char key_tmp = key&0xff;
			key_tmp = toupper( key_tmp );
			buffer[current]=key_tmp;
			current++;
			if(current==num_max){ //maximum char is reached
				current--;
				buffer[current]=0;
			}
		}
	}
clear_keybuf();
}

clear_to_color(back, makecol(255, 0, 255));
if(status==1) set_border(1);
else{
	set_border(0);
	set_option(0); // window not in use
}
outtextxy2(2, -4, buffer); //write text on background
//blit(back,tmp,0,0,0,0,w,h);
return status;//return_value;
}

//========================================================ANIMATED DWINDOW
ani_dwindow::ani_dwindow(int ix,int iy,int iw,int ih,BITMAP *f,BITMAP *aa[],int n){
//default constructor
x=ix;
y=iy;
w=iw;
h=ih;
back=create_bitmap(iw,ih);
tmp=create_bitmap(iw,ih);
fnt=f;
parent=NULL;
int i;
for(i=0;i<10;i++){s[i]=NULL;s_type[i]=0;}
n_siblings=0;

if(n>60)n=60;
n_frames=n;
for(i=0;i<n_frames;i++) a[i]=aa[i];
current=0;
back=a[0];
option=0;
}

//========================================================TEXT DWINDOW
txt_dwindow::txt_dwindow(int ix,int iy,int iw,int ih,BITMAP *f, char *tname){
//default constructor
x=ix;
y=iy;
w=iw;
h=ih;
back=create_bitmap(iw,ih);
tmp=create_bitmap(iw,ih);
fnt=f;
parent=NULL;
int i;
for(i=0;i<10;i++){s[i]=NULL;s_type[i]=0;}
n_siblings=0;

strcpy(name,tname);
showtime=50;
n_text=0;
for(i=0;i<20;i++){ strcpy(t[i],""); t_x[i]=0; t_y[i]=0;}

load_text();
clear_back();
print_text();
option=0;
showopt=1;
}

int txt_dwindow::load_text(){
//load text for this window from text.dat
fstream fs;
char tmp[30];
int num=0, exit=0;
fs.open("text.dat",ios::in);
if(!fs.is_open()) return 0;
while(!fs.eof()&&!exit){
	fs>>tmp;
	if(strcmp(tmp,"&")==0){
		fs>>tmp;
		if(strcmp(tmp,name)==0){
			fs>>num; //number of texts in the window should follow "& name"
			exit=1;
		}
	}
}
if(!exit) return 0; //no text with demanded name is found!

n_text=num;
for(int i=0;i<num;i++){
	fs>>t_x[i]>>t_y[i]; //first we get coords of text
	fs>>tmp; //then actual text followed by / char
	strcpy(t[i],"");
	while(strcmp(tmp,"/")!=0){
		strcat(t[i],tmp);
		strcat(t[i]," ");
		fs>>tmp;
	}
}
return 1;
}

void txt_dwindow::print_text(){
for(int i=0;i<n_text;i++)
	outtextxy(t_x[i],t_y[i],t[i]);
}

void txt_dwindow::showTxt_all(BITMAP *where){
if(n_siblings!=0)blit(back,tmp,0,0,0,0,w,h); //save back to tmp
else tmp=back;
int busy=0;

for(int i=0;i<n_siblings;i++){
	if(s_type[i]==0)s[i]->show_all(tmp);
	if(s_type[i]==1)((ani_dwindow*)s[i])->showAni(tmp);
	if(s_type[i]==2)((txt_dwindow*)s[i])->showTxt(tmp); //to save time we do not allow inheritance from text to text
	if(s_type[i]==3){
		if(s[i]->is_selected()&&busy==0){
			busy=1; //please do not use several inputs from different parents simultaniously
			((input_dwindow*)s[i])->set_status(1); //status=1 means this input box is currently in use
			((input_dwindow*)s[i])->set_border(1);
		}
		if(((input_dwindow*)s[i])->get_status()==1){
			((input_dwindow*)s[i])->input();
			blit(s[i]->back,s[i]->tmp,0,0,0,0,w,h);
		}
		else{
			((input_dwindow*)s[i])->set_border(0);
		}
		s[i]->show_all(tmp);
	}
}
showTxt(where);
}

void txt_dwindow::showtext(int frame,BITMAP *where){
//show animation of window appearing on parent window
int frames=50;
if(showopt==1){ //window is flying stretching from right-bottom corner to fill all parent window
	for(int i=0;i<w;i++)
	for(int j=0;j<h;j++){
		int col=getpixel(tmp,i,j);
		if(col!=makecol(255,0,255)){
			int x=where->w+float(frame*(i-where->w)/frames);
			int y=where->h+float(frame*(j-where->h)/frames);
			putpixel(where,x,y,col);
		}
	}
}
if(showopt==2){ //wobbling right-left and color cycling
	for(int i=0;i<w;i++)
	for(int j=0;j<h;j++){
		int col=getpixel(tmp,i,j);
		int r=getr(col);
		int g=getg(col);
		int b=getb(col);
		if(col!=makecol(255,0,255)){
			double PI = 3.1415;
			double cycle=sin(j/10.+2*PI*float(showtime)/50.);
			col=makecol(r,g*cycle,b);
			int x=i+sin(j/2.+frame);
			int y=j;
			putpixel(where,x,y,col);
		}
	}
if(showtime==50) showtime=0;
}
if(showopt==3){ //star-effect
	for(int i=0;i<w;i++)
	for(int j=0;j<h;j++){
		int col=getpixel(tmp,i,j);
		if(col!=makecol(255,0,255)){
			double PI = 3.1415;
			double cycle_s=sin(2*PI*float(showtime+i)/50.); //2-pi circle
			double cycle_c=cos(2*PI*float(showtime+j)/50.);
			double R=(50.-showtime)/5.;
			int x=i+R*cycle_s;
			int y=j+R*cycle_c;
			putpixel(where,x,y,col);
		}
	}
//if(showtime==50) showtime=0;
}
}

//==================================== LEGACY

/*void logtext(BITMAP* screen, BITMAP* font, char* text){
//special function for D&Z game. Shows log
for(int i=0;i<5;i++)
blit(screen,screen,5,470+(4-i)*20,5,470+(5-i)*20,544,21); //copy each previos log downwards

rectfill(screen, 5,470,544,490,makecol(50,0,0)); //clear top log window
rect    (screen, 5,470,544,490,makecol(200,0,0)); //shows frame to top log window
outtextxy(screen, font, 15,472, text); //prints text to it
}*/

/*
//"star" effect
void draw_char_star(BITMAP *where,BITMAP *fnt,int x, int y, char chr){
int func_x(int);
int func_y(int);
BITMAP * bmp_save  = create_bitmap(320,200);
BITMAP * bmp_save2 = create_bitmap(320,200);
blit(where,bmp_save,0,0,0,0,320,200); //save image from screen
unsigned char p=to_rus(chr);
int memx[8],memy[8],step[8];

for(int n=0;n<8;n++){
	memx[n]=0,memy[n]=0;step[n]=8-n;
}

int c=0;
while(c<8*15){
//for(int i=0;i<8;i++)
//for(int j=0;j<16;j++){
	for(int n=0;n<8;n++){
		int i=(c+n)%8;
		int j=(c+n)/8;

		int col=getpixel(fnt,(p%32)*8+i,(p/32)*16+j);
		if(col!=makecol(255,0,255)){
			putpixel(screen,x+i+func_x(n),y+j+func_y(n),col);
			if(n==7) putpixel(bmp_save,x+i+func_x(n),y+j+func_y(n),col);
		}
	}
rest(10);
blit(bmp_save,screen,0,0,0,0,320,200);
c++;
}
//blit(bmp_save,where,0,0,x,y,8,16);
//masked_blit(fnt,where,(p%32)*8,(p/32)*16,x,y,8,16);
}

int func_x(int t){
float R = 5, a=3.1415/10;
return R*sin(float(t)*a);
}
int func_y(int t){
float R = 5, a=3.1415/10;
return R*cos(float(t)*a-3.1415/2);
}*/
