#ifndef PATCH_H_INCLUDED
#define PATCH_H_INCLUDED

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <windows.h>
#include "crc32.h"

#define INITT 1
#define CHECKK 2
#define APPLY 3
#define DONE 4
#define ERRORR 5

typedef struct {
    unsigned long psize; // размер файла после применения патча
    unsigned long osize; // размер оригинального файла
    unsigned long crc32; // контрольная сумма оригинала
    unsigned long packed; // если упакован, то packed=размер упакованного файла, иначе = 0
} FileInfo;

class PatchClass
{
    private:
//	PatchInfo *Pinfo; // заголовок
        char description[128]; // краткое описание патча^M
        unsigned long num; // количество файлов^M

	unsigned char *mainbuf; // основной буфер
	unsigned long offset; // смещение буфера, на всякий случай mainbuf не трогаю
	unsigned long startoffset; // смещение начала данных
	unsigned char *srcbuf; // буфер
	unsigned char *destbuf; // буфер
	char regpath[128]; // путь к игре в реестре
	char regkey[32]; // ключ в котором хранится путь к игре на компьютере
	char gamepath[256]; // путь к игре
	char process; // текущий процесс
	int current; // текущий файл
	FileInfo FI; // текущий заголовок
	char currentpath[256]; // текущий путь
	bool nocrc; // отключить проверку CRC
	bool patchanyway; // ставить не смотря на несоответствие размеров
//	HWND hwnd; // окно

	
	void GetString(char *buf); // считывание строки из основного буфера, длина строки вместе с завершающим нолём - long
	bool CheckNextCRC(); // провериить CRC следующего файла
	bool PatchNext(); // патчить следующий файл
	void abortProcess() { // сброс параметров
		process=0;
		offset=startoffset;
		current=0;
	}
	void createDiff(unsigned long size); // применить патч

    public:

//PatchInfo *Pinfo;
	PatchClass(unsigned char *buf);
	bool getGamePath(char *buf);
	char *getCurrentPath() { return currentpath; }
	char *getTitle() { return description; }
	char getProcess() { return process; }
	bool check(char *path);
	~PatchClass();
};

#endif // PATCH_H_INCLUDED
