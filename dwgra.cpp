/*
Installer for translation releases.
Authors: Dimouse, Steel Rat
Old-Games.Ru Bureau, 2010-2012
*/

#define ALLEGRO_STATICLINK
#define ALLEGRO_USE_CONSOLE

//using namespace std;

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <sys/stat.h>

#include <allegro.h>
//#include "dwgra.h"
#include "dgui.h"
#include "minifmod.h"

#include "disc.h"
//#include "music.h"
#include "patch.h"

#define USEMEMLOAD
//#define USEMEMLOADRESOURCE

#define USEFMOD TRUE

#ifdef USEFMOD
	#include "minifmod.h"
#endif


// this is if you want to replace the samples with your own (in case you have compressed them)
void sampleloadcallback(void *buff, int lenbytes, int numbits, int instno, int sampno)
{
	printf("pointer = %p length = %d bits = %d instrument %d sample %d\n", buff, lenbytes, numbits, instno, sampno);
}



#ifndef USEMEMLOAD

unsigned int fileopen(char *name)
{
	return (unsigned int)fopen(name, "rb");
}

void fileclose(unsigned int handle)
{
	fclose((FILE *)handle);
}

int fileread(void *buffer, int size, unsigned int handle)
{
	return fread(buffer, 1, size, (FILE *)handle);
}

void fileseek(unsigned int handle, int pos, signed char mode)
{
	fseek((FILE *)handle, pos, mode);
}

int filetell(unsigned int handle)
{
	return ftell((FILE *)handle);
}

#else

typedef struct 
{
	int length;
	int pos;
	void *data;
} MEMFILE;


unsigned int memopen(char *name)
{
	MEMFILE *memfile;

	memfile = (MEMFILE *)calloc(sizeof(MEMFILE),1);

#ifndef USEMEMLOADRESOURCE
	{	// load an external file and read it
		FILE *fp;
		fp = fopen(name, "rb");
		if (fp)
		{
			fseek(fp, 0, SEEK_END);
			memfile->length = ftell(fp);
			memfile->data = calloc(memfile->length,1);
			memfile->pos = 0;

			fseek(fp, 0, SEEK_SET);
			fread(memfile->data, 1, memfile->length, fp);
			fclose(fp);
		}
	}
#else
	{	// hey look some load from resource code!
		HRSRC		rec;
		HGLOBAL		handle;

		rec = FindResource(NULL, name, RT_RCDATA);
		handle = LoadResource(NULL, rec);
		
		memfile->data = LockResource(handle);
		memfile->length = SizeofResource(NULL, rec);
		memfile->pos = 0;
	}
#endif

	return (unsigned int)memfile;
}

void memclose(unsigned int handle)
{
	MEMFILE *memfile = (MEMFILE *)handle;

#ifndef USEMEMLOADRESOURCE
	free(memfile->data);			// dont free it if it was initialized with LockResource
#endif

	free(memfile);
}

int memread(void *buffer, int size, unsigned int handle)
{
	MEMFILE *memfile = (MEMFILE *)handle;

	if (memfile->pos + size >= memfile->length)
		size = memfile->length - memfile->pos;

	memcpy(buffer, (char *)memfile->data+memfile->pos, size);
	memfile->pos += size;
	
	return size;
}

void memseek(unsigned int handle, int pos, signed char mode)
{
	MEMFILE *memfile = (MEMFILE *)handle;

	if (mode == SEEK_SET) 
		memfile->pos = pos;
	else if (mode == SEEK_CUR) 
		memfile->pos += pos;
	else if (mode == SEEK_END)
		memfile->pos = memfile->length + pos;

	if (memfile->pos > memfile->length)
		memfile->pos = memfile->length;
}

int memtell(unsigned int handle)
{
	MEMFILE *memfile = (MEMFILE *)handle;

	return memfile->pos;
}
#endif

//=========================================GLOBAL VARIABLES
int size_x=640;
int size_y=400;
int fullscreen=0;
int textmode=0;
int errorpath=0;
int effects=1;
char path[512]="C:/DW";
char musicfile[512]="roadsea.xm";
int playmusic=1;
char gfxfile[512]="disc.dat";
char patchfile[512]="disc.pat";

void makeDir(char name[255])
{
	#ifdef linux
	mkdir(name, S_IRWXU | S_IRWXG | S_IRWXO);
	#endif
	#ifdef _MSC_VER
	mkdir(name);
	#endif
}

//=========================================MAIN
int main(){
	int set_options();
	int blit2(BITMAP *source, BITMAP *dest, int source_x,int source_y,int source_w,int source_h, int dest_x,int dest_y,int dest_w,int dest_h);
	int masked_blit2(BITMAP *source, BITMAP *dest, int source_x,int source_y,int source_w,int source_h, int dest_x,int dest_y,int dest_w,int dest_h);

	printf("Begin initialization\n");

//Setting all global variables
	if(!set_options()){
		printf("Can't set up options, check if you have install.cfg\n");
		return 0;
	}

	printf("Options loaded from install.cfg\n");
	
	PatchClass *patch;

	printf("Patch created\n");

	if (allegro_init()) {
		printf("Cannot initalize Allegro.\n");
	    return 0;
	}
	install_timer();
	install_mouse();
	install_keyboard();
	set_color_depth(32);

	printf("Allegro initialized\n");
	
//Open and initialize patch
	FILE * pFile;
	long lSize;
	unsigned char * buffer;
	size_t result;

	pFile = fopen (patchfile,"rb");

// obtain file size:
	if(pFile){
	  fseek (pFile , 0 , SEEK_END);
	  lSize = ftell (pFile);
	  rewind (pFile);

	  printf("Size of patch: %d\n",lSize);

// allocate memory to contain the whole file:
	  buffer = (unsigned char*) malloc (sizeof(unsigned char)*lSize);

// copy the file into the buffer:
	  result = fread (buffer,1,lSize,pFile);

/* the whole file is now loaded in the memory buffer. */
	
/*	int handle1 = open(patchfile, O_CREAT | O_RDWR, S_IREAD | S_IWRITE);
	if(handle1){
		lseek(handle1, 0, SEEK_SET);
		unsigned char *ch = (unsigned char *) malloc(filelength(handle1));
		read(handle1,ch,filelength(handle1));
*/			

      patch=new PatchClass(buffer);				
      printf("Patch loaded into memory\n");
	}
	else{
      printf("Can't open patch file, exiting\n");
      return 0;
	}

//Set Gfx mode if all is fine

    printf("Setting gfx...\n");
	if(size_x==0||size_y==0){//Bad screen size parameters
		allegro_exit();
		printf("No gfx mode with this parameters\n");
		textmode=1;
	}

	if(textmode==0){ //Gfx data is found
		int setgfxmode=0;
		
		if(fullscreen)setgfxmode=set_gfx_mode(GFX_AUTODETECT_FULLSCREEN,size_x,size_y,0,0);
		else setgfxmode=set_gfx_mode(GFX_AUTODETECT_WINDOWED,size_x,size_y,0,0);
		
		if(setgfxmode!=0){ //Can't initialize gfx mode
			printf("Problem setting gfx mode\n");
			textmode=1;
		}
	}
	if(textmode==1){ //If textmode is on install patch in text mode
		//allegro_exit();
		printf("Entering textmode install\n");
		unsigned char proc=0;
		if(errorpath){
			printf("Check if directory specified in cfg exist\n");
			return 0;
		}
		patch->check(path);
		//printf("Path: %s\n",path);
		while(proc!=DONE&&proc!=ERRORR){
			patch->check(path);
			proc=patch->getProcess();
			patch->getGamePath(path);
			printf("Path: %s\n",patch->getCurrentPath());
			//strcat(path, patch->getCurrentPath());
			if(proc==INITT) printf("Initializing %s\n",patch->getCurrentPath());
			if(proc==CHECKK) printf("Checking %s\n",patch->getCurrentPath());
			if(proc==APPLY) printf("Applying %s\n",patch->getCurrentPath());
			if(proc==DONE) printf("All is done!\n");
                        if(proc==ERRORR) printf("Error\n");
		}
		delete patch;
        char tmppath[256];
        strcpy(tmppath,path);
        strcat(tmppath,"/SAVE");
        makeDir(tmppath);
	return 1;
	}

//Music routines
	#ifdef USEFMOD
		FMUSIC_MODULE *mod;

	#ifndef USEMEMLOAD
		FSOUND_File_SetCallbacks(fileopen, fileclose, fileread, fileseek, filetell);
	#else
		FSOUND_File_SetCallbacks(memopen, memclose, memread, memseek, memtell);
	#endif

    #endif

	#ifdef USEFMOD
		mod = FMUSIC_LoadSong(musicfile, NULL); //sampleloadcallback);
		if (!mod)
		{
			printf("Error loading song\n");
			return 0;
		}
	if(playmusic)
		FMUSIC_PlaySong(mod);
	#endif


DATAFILE *data = load_datafile(gfxfile);

if(data==NULL) return 0;

BITMAP * back1  = (BITMAP*)(data[BACK].dat);//create_bitmap(320,200);
BITMAP * title  = (BITMAP*)(data[TITLE].dat);//create_bitmap(320,52);
BITMAP * fnt1   = (BITMAP*)(data[FONTT].dat);//create_bitmap(256,128);
//BITMAP * fnt2   = (BITMAP*)(data[FONT2].dat);
BITMAP * chkbox1  = (BITMAP*)(data[CHKBOX1].dat);//create_bitmap(23,19);
BITMAP * chkbox2  = (BITMAP*)(data[CHKBOX2].dat);//create_bitmap(23,19);
BITMAP * closebox  = (BITMAP*)(data[CLOSE].dat);//create_bitmap(23,19);
BITMAP * cancelbox  = (BITMAP*)(data[CANCEL].dat);//create_bitmap(23,19);
BITMAP * a[61], * b[61];

a[ 0]= (BITMAP*)(data[T001].dat);
a[ 1]= (BITMAP*)(data[T002].dat);
a[ 2]= (BITMAP*)(data[T003].dat);
a[ 3]= (BITMAP*)(data[T004].dat);
a[ 4]= (BITMAP*)(data[T005].dat);
a[ 5]= (BITMAP*)(data[T006].dat);
a[ 6]= (BITMAP*)(data[T007].dat);
a[ 7]= (BITMAP*)(data[T008].dat);
a[ 8]= (BITMAP*)(data[T010].dat);
a[ 9]= (BITMAP*)(data[T012].dat);
a[10]= (BITMAP*)(data[T013].dat);
a[11]= (BITMAP*)(data[T012].dat);
a[12]= (BITMAP*)(data[T010].dat);
a[13]= (BITMAP*)(data[T008].dat);
a[14]= (BITMAP*)(data[T007].dat);
a[15]= (BITMAP*)(data[T006].dat);
a[16]= (BITMAP*)(data[T005].dat);
a[17]= (BITMAP*)(data[T004].dat);
a[18]= (BITMAP*)(data[T003].dat);
a[19]= (BITMAP*)(data[T002].dat);
a[20]= (BITMAP*)(data[T001].dat);
a[21]= (BITMAP*)(data[T002].dat);
a[22]= (BITMAP*)(data[T003].dat);
a[23]= (BITMAP*)(data[T004].dat);
a[24]= (BITMAP*)(data[T005].dat);
a[25]= (BITMAP*)(data[T006].dat);
a[26]= (BITMAP*)(data[T007].dat);
a[27]= (BITMAP*)(data[T008].dat);
a[28]= (BITMAP*)(data[T010].dat);
a[29]= (BITMAP*)(data[T012].dat);
a[30]= (BITMAP*)(data[T013].dat);
a[31]= (BITMAP*)(data[T012].dat);
a[32]= (BITMAP*)(data[T010].dat);
a[33]= (BITMAP*)(data[T008].dat);
a[34]= (BITMAP*)(data[T007].dat);
a[35]= (BITMAP*)(data[T006].dat);
a[36]= (BITMAP*)(data[T005].dat);
a[37]= (BITMAP*)(data[T004].dat);
a[38]= (BITMAP*)(data[T003].dat);
a[39]= (BITMAP*)(data[T002].dat);
a[40]= (BITMAP*)(data[T001].dat);
a[41]= (BITMAP*)(data[T002].dat);
a[42]= (BITMAP*)(data[T003].dat);
a[43]= (BITMAP*)(data[T004].dat);
a[44]= (BITMAP*)(data[T005].dat);
a[45]= (BITMAP*)(data[T006].dat);
a[46]= (BITMAP*)(data[T007].dat);
a[47]= (BITMAP*)(data[T009].dat);
a[48]= (BITMAP*)(data[T011].dat);
a[49]= (BITMAP*)(data[T012].dat);
a[50]= (BITMAP*)(data[T013].dat);
a[51]= (BITMAP*)(data[T012].dat);
a[52]= (BITMAP*)(data[T011].dat);
a[53]= (BITMAP*)(data[T009].dat);
a[54]= (BITMAP*)(data[T007].dat);
a[55]= (BITMAP*)(data[T006].dat);
a[56]= (BITMAP*)(data[T005].dat);
a[57]= (BITMAP*)(data[T004].dat);
a[58]= (BITMAP*)(data[T003].dat);
a[59]= (BITMAP*)(data[T002].dat);

b[ 0]= (BITMAP*)(data[S030].dat);
b[ 1]= (BITMAP*)(data[S029].dat);
b[ 2]= (BITMAP*)(data[S028].dat);
b[ 3]= (BITMAP*)(data[S027].dat);
b[ 4]= (BITMAP*)(data[S026].dat);
b[ 5]= (BITMAP*)(data[S025].dat);
b[ 6]= (BITMAP*)(data[S024].dat);
b[ 7]= (BITMAP*)(data[S023].dat);
b[ 8]= (BITMAP*)(data[S022].dat);
b[ 9]= (BITMAP*)(data[S021].dat);
b[10]= (BITMAP*)(data[S020].dat);
b[11]= (BITMAP*)(data[S019].dat);
b[12]= (BITMAP*)(data[S018].dat);
b[13]= (BITMAP*)(data[S017].dat);
b[14]= (BITMAP*)(data[S016].dat);
b[15]= (BITMAP*)(data[S015].dat);
b[16]= (BITMAP*)(data[S014].dat);
b[17]= (BITMAP*)(data[S013].dat);
b[18]= (BITMAP*)(data[S012].dat);
b[19]= (BITMAP*)(data[S011].dat);
b[20]= (BITMAP*)(data[S010].dat);
b[21]= (BITMAP*)(data[S009].dat);
b[22]= (BITMAP*)(data[S008].dat);
b[23]= (BITMAP*)(data[S007].dat);
b[24]= (BITMAP*)(data[S006].dat);
b[25]= (BITMAP*)(data[S005].dat);
b[26]= (BITMAP*)(data[S004].dat);
b[27]= (BITMAP*)(data[S003].dat);
b[28]= (BITMAP*)(data[S002].dat);
b[29]= (BITMAP*)(data[S001].dat);
b[30]= (BITMAP*)(data[S030].dat);
b[31]= (BITMAP*)(data[S029].dat);
b[32]= (BITMAP*)(data[S028].dat);
b[33]= (BITMAP*)(data[S027].dat);
b[34]= (BITMAP*)(data[S026].dat);
b[35]= (BITMAP*)(data[S025].dat);
b[36]= (BITMAP*)(data[S024].dat);
b[37]= (BITMAP*)(data[S023].dat);
b[38]= (BITMAP*)(data[S022].dat);
b[39]= (BITMAP*)(data[S021].dat);
b[40]= (BITMAP*)(data[S020].dat);
b[41]= (BITMAP*)(data[S019].dat);
b[42]= (BITMAP*)(data[S018].dat);
b[43]= (BITMAP*)(data[S017].dat);
b[44]= (BITMAP*)(data[S016].dat);
b[45]= (BITMAP*)(data[S015].dat);
b[46]= (BITMAP*)(data[S014].dat);
b[47]= (BITMAP*)(data[S013].dat);
b[48]= (BITMAP*)(data[S012].dat);
b[49]= (BITMAP*)(data[S011].dat);
b[50]= (BITMAP*)(data[S010].dat);
b[51]= (BITMAP*)(data[S009].dat);
b[52]= (BITMAP*)(data[S008].dat);
b[53]= (BITMAP*)(data[S007].dat);
b[54]= (BITMAP*)(data[S006].dat);
b[55]= (BITMAP*)(data[S005].dat);
b[56]= (BITMAP*)(data[S004].dat);
b[57]= (BITMAP*)(data[S003].dat);
b[58]= (BITMAP*)(data[S002].dat);
b[59]= (BITMAP*)(data[S001].dat);

//clear_to_color(title,0);

//set_color_conversion(COLORCONV_TOTAL | COLORCONV_KEEP_TRANS); 

BITMAP * back2 = create_bitmap(320,200);
BITMAP * back3 = create_bitmap(320,200);

int exit=0;
int install_opt =0; //which terms
int install_opt2=0; //with or without dosbox
int page=0; //first page - main menu, second - install options, third - results of intallation and credits

//create windows
dwindow *mainw = new dwindow(0,0,320,200,fnt1,back1); //galaxy background
mainw->create_ani_sibling(0,0,320,200,b,60); //animation of stars
mainw->create_ani_sibling(9,18,309,170,a,60); //animation of turtle
mainw->create_sibling(0,0,320,200,title); //title
mainw->create_txt_sibling(0,0,320,200,"main"); //text
if(effects) ((txt_dwindow*)(mainw->s[3]))->set_time(0); //set appearing animation to all 50 frames
if(effects) ((txt_dwindow*)(mainw->s[3]))->set_showopt(1); //set appearing animation to all 50 frames
mainw->s[3]->create_sibling(0,140,23,19,chkbox1);
//mainw->s[3]->create_sibling(0,120,23,19,chkbox1);
mainw->s[3]->create_sibling(5,187,35,10,cancelbox);

while(!exit){ //show everything
	mainw->show_all(back2);
	blit2(back2,screen,0,0,320,200,0,0,size_x,size_y);

	if(install_opt!=0&&page==0){ //some button chosen at page 0
		page=1;
		mainw->delete_sibling(3);
		mainw->create_txt_sibling(0,0,320,200,"options"); //text
		if(effects) ((txt_dwindow*)(mainw->s[3]))->set_time(0); //set appearing animation to all 50 frames
		if(effects) ((txt_dwindow*)(mainw->s[3]))->set_showopt(3); //set appearing animation to all 50 frames
		mainw->s[3]->create_sibling(25,140,23,19,chkbox1);
		//mainw->s[3]->create_sibling(25,160,23,19,chkbox1);
		mainw->s[3]->create_sibling(5,187,35,10,cancelbox);
		//mainw->s[3]->create_inp_sibling(139,69,31,11,1,2);
		mainw->s[3]->create_inp_sibling(0,99,319,11,3,39);
		if(!errorpath){
			((input_dwindow*)(mainw->s[3]->s[2]))->set_buffer(path); //Set path from cfg to input dwindow
			((txt_dwindow*)(mainw->s[3]))->set_text(2,"Корректный путь ");
		}
	}
	if(page==1){ //some button chosen at page 1
		if(((input_dwindow*)(mainw->s[3]->s[2]))->get_status()==2){ //check path
			//get path from input dwindow
			strcpy(path,((input_dwindow*)(mainw->s[3]->s[2]))->get_buffer());
			
//Attempting to create file in the path specified. If created - then it's good path
			char tmpname[256]="/TEST.TMP";
			char tmppath[256];
			strcpy(tmppath,path);
			strcat(tmppath,tmpname);
			FILE *ff=fopen(tmppath,"wb");
			if(!ff){
				errorpath=1;
				((txt_dwindow*)(mainw->s[3]))->set_text(2,"Некорректный путь ");
				install_opt2=0;
				mainw->s[3]->s[0]->set_back(chkbox1);
				mainw->s[3]->s[0]->set_option(0);
				}
			else{
				errorpath=0;
				((txt_dwindow*)(mainw->s[3]))->set_text(2,"Корректный путь ");
				fclose(ff);
				remove(tmppath);
			}
		}
		if(install_opt2!=0){ //if install button pressed
			if(!errorpath){ //if path is good
				char proc=0;
				char oldproc=5;
				int installed=0;
				patch->check(path);
				while(proc!=DONE&&proc!=ERRORR){ //do patching...
					patch->check(path);
					proc=patch->getProcess();
					patch->getGamePath(path);

					char pstatus[512]="";
				    if(proc==INITT)sprintf(pstatus,"Инициализация %s ",patch->getCurrentPath());
				    if(proc==CHECKK)sprintf(pstatus,"Проверка %s ",patch->getCurrentPath());
				    if(proc==APPLY)sprintf(pstatus,"Применение %s ",patch->getCurrentPath());
					if(proc==DONE) { sprintf(pstatus,"Готово "); installed=1;}
					if(proc==ERRORR) { sprintf(pstatus,"Ошибка "); installed=0;}
					((txt_dwindow*)(mainw->s[3]))->set_text(2,pstatus);

					mainw->show_all(back2);
					blit2(back2,screen,0,0,320,200,0,0,size_x,size_y);
					rest(60);

					//strcat(path, patch->getCurrentPath());
				}
				delete patch;

                char tmppath[256];
                strcpy(tmppath,path);
                strcat(tmppath,"/SAVE");
                makeDir(tmppath);

				//show final page
				page=2;
				mainw->delete_sibling(3);
				if(installed) mainw->create_txt_sibling(0,0,320,200,"result1"); //text
				else mainw->create_txt_sibling(0,0,320,200,"result2"); //text
				mainw->create_txt_sibling(0,0,320,200,"credits"); //text		
				if(effects) ((txt_dwindow*)(mainw->s[4]))->set_time(0); //set appearing animation to all 50 frames
				if(effects) ((txt_dwindow*)(mainw->s[4]))->set_showopt(2); //set appearing animation to all 50 frames
				mainw->s[3]->create_sibling(5,187,35,10,closebox);
			}
			else{ //no good path
				install_opt2=0;
				mainw->s[3]->s[0]->set_back(chkbox1);
				mainw->s[3]->s[0]->set_option(0);
			}
		}
	}

	if(keypressed()){
		int key = readkey();
		if (key>>8 == KEY_ESC) exit=1;
	//	clear_keybuf();
	}
	show_mouse(screen);
	if(page==0){
		if(mainw->s[3]->is_selected_sib(0)) {mainw->s[3]->s[0]->set_back(chkbox2); install_opt=1;}
		//if(mainw->s[3]->is_selected_sib(1)) {mainw->s[3]->s[1]->set_back(chkbox2); install_opt=2;}
		if(mainw->s[3]->is_selected_sib(1)) exit=1;
	}
	if(page==1){
		if(mainw->s[3]->is_selected_sib(0)) { // intall button pressed
			mainw->s[3]->s[0]->set_back(chkbox2);
			//mainw->s[3]->s[1]->set_back(chkbox1);
			//mainw->s[3]->s[1]->set_option(0);
			install_opt2=1;
		}
		/*if(mainw->s[3]->is_selected_sib(1)) {
			mainw->s[3]->s[1]->set_back(chkbox2);
			mainw->s[3]->s[0]->set_back(chkbox1);
			mainw->s[3]->s[0]->set_option(0);
			install_opt2=2;
		}*/
		if(mainw->s[3]->is_selected_sib(1)) exit=1;
	}
	if(page==2){
		if(mainw->s[3]->is_selected_sib(0)) exit=1;
	}

	rest(60);
	//clear_keybuf();
	scare_mouse();
}

#ifdef USEFMOD
	if(playmusic) FMUSIC_FreeSong(mod);
#endif

//set_gfx_mode(GFX_TEXT,640,480,0,0);
fclose (pFile);
free (buffer);

allegro_exit();
return 1;
}

END_OF_MAIN();

//=======================================================GLOBAL FUNCTIONS
int blit2(BITMAP *source, BITMAP *dest, int source_x,int source_y,int source_w,int source_h, int dest_x,int dest_y,int dest_w,int dest_h){
	if(size_x==320&&size_y==200) blit(source,dest,source_x,source_y,dest_x,dest_y,source_w,source_h);
	else stretch_blit(source,dest,source_x,source_y,source_w,source_h,dest_x,dest_y,dest_w,dest_h);
	return 1;
}

int masked_blit2(BITMAP *source, BITMAP *dest, int source_x,int source_y,int source_w,int source_h, int dest_x,int dest_y,int dest_w,int dest_h){
	if(size_x==320&&size_y==200) masked_blit(source,dest,source_x,source_y,dest_x,dest_y,source_w,source_h);
	else masked_stretch_blit(source,dest,source_x,source_y,source_w,source_h,dest_x,dest_y,dest_w,dest_h);
	return 1;
}

//=============================================Set Options!
int set_options(){
	FILE *f=fopen("install.cfg","rb");
	char val[200];
	char var[200];

//	fstream fdebug;
//fdebug.open("dddebug.txt",ios::out);

	int n=0;
	strcpy(val,"");
	strcpy(var,"");
	if(!f) return 0;

	while(!feof(f)){
		unsigned char c=fgetc(f);
		if(c==';'){ //comment
			while(c!=0x0A){
				c=fgetc(f);
				if(feof(f))break;
			}
		}
		else{
			strcpy(val,"");
			strcpy(var,"");
			n=0;
			while(c!=':'){
				if(feof(f))break;
				var[n]=c;
				c=fgetc(f);
				n++;
			}
			var[n]=0;
			c=fgetc(f); //space
			n=0;
			c=fgetc(f);
			while(c!=0x0D){
				val[n]=c;
				c=fgetc(f);		
				if(feof(f))break;
				n++;
			}
			val[n]=0;
			c=fgetc(f); //0x0A
			if(feof(f))break;
//			fdebug<<var<<" "<<val<<endl;
			if(strcmp(var,"Width")==0){
				int width=atoi(val);
				if(width>0 && width <= 1280) size_x=width;
				else size_x=0;
			}
			if(strcmp(var,"Height")==0){
				int height=atoi(val);
				if(height>0 && height <= 800) size_y=height;
				else size_y=0;
			}
			if(strcmp(var,"Fullscreen")==0){
				if(strcmp(val,"On")==0) fullscreen=1;
				if(strcmp(val,"Off")==0) fullscreen=0;
			}
			if(strcmp(var,"Gfxdata")==0){
				strcpy(gfxfile,val);
				FILE *ff=fopen(gfxfile,"rb");
				if(!ff) textmode=1;
				else{
					textmode=0;
					fclose(ff);
				}
			}
			if(strcmp(var,"Musax")==0){
				if(strcmp(val,"None")==0) playmusic=0;
				else {
					playmusic=1;
					strcpy(musicfile,val);
				}
			}
			if(strcmp(var,"Effects")==0){
				if(strcmp(val,"On")==0) effects=1;
				if(strcmp(val,"Off")==0) effects=0;
			}
			if(strcmp(var,"Install")==0){
				strcpy(path,val);
				char tmpname[256]="/TEST.TMP";
				char tmppath[256];
				strcpy(tmppath,path);
				strcat(tmppath,tmpname);
				FILE *ff=fopen(tmppath,"wb");
				if(!ff){
					//printf("Problems with creating file to this dir\n");
					//strcpy(path,"C:/DW");
					errorpath=1;
				}
				else{
					//printf("File created ok\n");
					fclose(ff);
					remove(tmppath);
				}
			}
		}
	}
//	fdebug.close();
	fclose(f);
	return 1;
}
