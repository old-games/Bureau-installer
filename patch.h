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
    unsigned long psize; // ������ ����� ����� ���������� �����
    unsigned long osize; // ������ ������������� �����
    unsigned long crc32; // ����������� ����� ���������
    unsigned long packed; // ���� ��������, �� packed=������ ������������ �����, ����� = 0
} FileInfo;

class PatchClass
{
    private:
//	PatchInfo *Pinfo; // ���������
        char description[128]; // ������� �������� �����^M
        unsigned long num; // ���������� ������^M

	unsigned char *mainbuf; // �������� �����
	unsigned long offset; // �������� ������, �� ������ ������ mainbuf �� ������
	unsigned long startoffset; // �������� ������ ������
	unsigned char *srcbuf; // �����
	unsigned char *destbuf; // �����
	char regpath[128]; // ���� � ���� � �������
	char regkey[32]; // ���� � ������� �������� ���� � ���� �� ����������
	char gamepath[256]; // ���� � ����
	char process; // ������� �������
	int current; // ������� ����
	FileInfo FI; // ������� ���������
	char currentpath[256]; // ������� ����
	bool nocrc; // ��������� �������� CRC
	bool patchanyway; // ������� �� ������ �� �������������� ��������
//	HWND hwnd; // ����

	
	void GetString(char *buf); // ���������� ������ �� ��������� ������, ����� ������ ������ � ����������� ���� - long
	bool CheckNextCRC(); // ���������� CRC ���������� �����
	bool PatchNext(); // ������� ��������� ����
	void abortProcess() { // ����� ����������
		process=0;
		offset=startoffset;
		current=0;
	}
	void createDiff(unsigned long size); // ��������� ����

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
