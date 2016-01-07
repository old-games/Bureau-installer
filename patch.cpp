#include "patch.h"
#include <fstream.h>

// Author: Steel Rat, 2010

PatchClass::PatchClass(unsigned char *buf)
{
    process=0;
    current=0;
//    Pinfo=(PatchInfo *) buf;   // сначала в файле патча идёт заголовок   (1)
memcpy(description, buf,128*sizeof(char));
unsigned char size1[4];
memcpy(&size1,buf+128,4*sizeof(unsigned char));
unsigned long nnum=size1[3]*256*256*256+size1[2]*256*256+size1[1]*256+size1[0];
num=nnum;
//printf("Patchinfo size=%d num=%d\n",sizeof(PatchInfo),nnum); 
//printf("Patchinfo size=%d name=%s\n",sizeof(PatchInfo),Pinfo->description);

    mainbuf=buf;
    offset=128+4;//sizeof(PatchInfo);
    GetString(regpath);         // затем идёт путь в реестре к переменной игры      (2)
    GetString(regkey);          // имя переменной с путём к игре                    (3)
    startoffset=offset;
    gamepath[0]=0;

    srcbuf=(unsigned char *) malloc(1024);
    destbuf=(unsigned char *) malloc(1024);

    nocrc=false;
    patchanyway=false;
}

PatchClass::~PatchClass()
{
    if (srcbuf!=NULL) free(srcbuf);
    if (destbuf!=NULL) free(destbuf);
}

void PatchClass::GetString(char *buf)
{
    unsigned long size;
    unsigned char size1[4];

    buf[0]=0; // на всякий случай, вдруг size будет 0
//    memcpy(&size, (mainbuf+offset), sizeof(size));
memcpy(&size1,(mainbuf+offset),4);
size=size1[3]*256*256*256+size1[2]*256*256+size1[1]*256+size1[0];
//printf("size=%d sizeof=%d, address=%d\n",size,sizeof(size),*(mainbuf+offset+3));
    offset+=sizeof(size);
    memcpy(buf, (mainbuf+offset), size);
    offset+=size;
}

bool PatchClass::getGamePath(char *buf)
{
    if (strlen(gamepath)==0) return false;
    strcpy(buf, gamepath);
    return true;
}

bool PatchClass::check(char *path)
{
    // это место надо переработать
    int len=strlen(path);

    if ((len>1)&&(process==0)) {
        ++process;
    }

    switch (process) {

    case INITT:
        strcpy(gamepath, path);
        --len;
        // если на конце \ - удаляем его, patchmaker должен был создать имена со слэшем

        if (gamepath[len]==0x5C) gamepath[len]=0;
        ++process;
        nocrc=false;
        return true;
        break;

    case CHECKK:

        if (!CheckNextCRC()){
//			abortProcess();
printf("CRC not equal\n");
			process=ERRORR;
		}
        return true;
        break;

    case APPLY:

        if (!PatchNext()) {
			process=ERRORR;
//            MessageBox(hwnd, "Error while patching!\nGame files maybe corrupted!\nProcess continues...", "Error!", MB_OK);
       }

        return true;
        break;

    case DONE:

        return true;
        break;

    case ERRORR:

        return true;
        break;

    }

    return false;
}

bool PatchClass::CheckNextCRC()
{
    char tmp[512];
    unsigned long psize;

    // закончить проверку по окончанию, перейти к применению патча
    if (current>=num) {
        current=0;
        offset=startoffset;
        ++process;
        return true;
    };
    // копируем FileInfo из памяти              (4)

//    memcpy(&FI, mainbuf+offset, sizeof(FileInfo));
   unsigned char size1[4];

memcpy(&size1,(mainbuf+offset),4);
FI.psize=size1[3]*256*256*256+size1[2]*256*256+size1[1]*256+size1[0];
offset+=4;
memcpy(&size1,(mainbuf+offset),4);
FI.osize=size1[3]*256*256*256+size1[2]*256*256+size1[1]*256+size1[0];
offset+=4;
memcpy(&size1,(mainbuf+offset),4);
FI.crc32=size1[3]*256*256*256+size1[2]*256*256+size1[1]*256+size1[0];
offset+=4;
memcpy(&size1,(mainbuf+offset),4);
FI.packed=size1[3]*256*256*256+size1[2]*256*256+size1[1]*256+size1[0];
offset+=4;

//    offset+=sizeof(FileInfo);

    // считываем имя файла                      (5)

    GetString(tmp);
    strcpy(currentpath, gamepath);

//printf("psize=%d osize=%d crc32=%d packed=%d filename=%s\n",FI.psize,FI.osize,FI.crc32,FI.packed,tmp);
if(tmp[0]=='\\')tmp[0]='/';

    strcat(currentpath, tmp);

    // затем идёт размер данных патча           (6)
//   unsigned char size1[4];

memcpy(&size1,(mainbuf+offset),4);
psize=size1[3]*256*256*256+size1[2]*256*256+size1[1]*256+size1[0];

//    memcpy(&psize, mainbuf+offset, sizeof(psize));

    // если размер оригинала - 0, то файла не было в оригинале, копируется целиком
    offset+=psize+sizeof(psize);

    if (FI.osize==0) {
        ++current;
        return true;
    }
    FILE *file;
    file=fopen(currentpath, "rb");
    if (file==NULL) {
//        sprintf(tmp, "%s:\nUnable to open!", currentpath);
//        MessageBox(hwnd, tmp, "Error!", MB_OK);
printf("Unable to open %s\n", currentpath);
        return false;
    }
    fseek(file, 0, SEEK_END);
    long fsize=ftell(file);
    fseek(file, 0, SEEK_SET);
    if ((!patchanyway)&&(fsize!=FI.osize)) {
        printf("Size is not equal to original: your size: %d, original: %d\n",fsize,FI.osize);
        //sprintf(tmp, "%s:\nFile size is not equal to original!\nYour size: %d, original size: %d\nPatch game ANY WAY? (NO recommended)", currentpath, fsize, FI.osize);
        //if (MessageBox(hwnd, tmp, "Error!", MB_YESNO)==IDYES) patchanyway=true;
        //   else {
                fclose(file);
                return false;
          //  }
    }
    srcbuf=(unsigned char *) realloc(srcbuf, fsize);
    fread(srcbuf, fsize, 1, file);
    fclose(file);
    if ((!nocrc)&&(Crc32(srcbuf, fsize)!=FI.crc32)) {
//        sprintf(tmp, "%s:\nBad CRC! Your: %X, original: %X\nTurn off CRC check? (NO recommended)", currentpath, Crc32(srcbuf, fsize),  FI.crc32);
//        if (MessageBox(hwnd, tmp, "Error!", MB_YESNO)==IDYES) nocrc=true;
//            else
				return false;
    }
    ++current;
    return true;
}

//======================================PATCHING
bool PatchClass::PatchNext(){
    char tmp[512];
    unsigned long patchsize=0;
    FILE *file;
    if (current>=num) {
        abortProcess();
        process=DONE;
        return true;
    };

unsigned char size1[4];

memcpy(&size1,(mainbuf+offset),4);
FI.psize=size1[3]*256*256*256+size1[2]*256*256+size1[1]*256+size1[0];
offset+=4;
memcpy(&size1,(mainbuf+offset),4);
FI.osize=size1[3]*256*256*256+size1[2]*256*256+size1[1]*256+size1[0];
offset+=4;
memcpy(&size1,(mainbuf+offset),4);
FI.crc32=size1[3]*256*256*256+size1[2]*256*256+size1[1]*256+size1[0];
offset+=4;
memcpy(&size1,(mainbuf+offset),4);
FI.packed=size1[3]*256*256*256+size1[2]*256*256+size1[1]*256+size1[0];
offset+=4;

//    memcpy(&FI, mainbuf+offset, sizeof(FileInfo));
//    offset+=sizeof(FileInfo);
    GetString(tmp);
if(tmp[0]=='\\')tmp[0]='/';

    strcpy(currentpath, gamepath);
    strcat(currentpath, tmp);

//   unsigned char size1[4];

memcpy(&size1,(mainbuf+offset),4);
patchsize=size1[3]*256*256*256+size1[2]*256*256+size1[1]*256+size1[0];

//    memcpy(&patchsize, mainbuf+offset, sizeof(patchsize));

    offset+=sizeof(patchsize);
    // если размер оригинала - 0, то файла не было в оригинале, копируется целиком
    if (FI.osize==0) {
        file=fopen(currentpath, "wb");
        fwrite(mainbuf+offset, FI.psize, 1, file);
        fclose(file);
        offset+=FI.psize;
        ++current;
        return true;
    }
    srcbuf=(unsigned char *) realloc(srcbuf, patchsize);
    destbuf=(unsigned char *) realloc(destbuf, FI.psize);
    //
    memcpy(srcbuf, mainbuf+offset, patchsize);
    offset+=patchsize;
    //
    file=fopen(currentpath, "rb");
    if (file==NULL) {
//        sprintf(tmp, "%s:\nUnable to open!", currentpath);
//        MessageBox(hwnd, tmp, "Error!", MB_OK);
          printf("Unable to open!\n");
        return false;
    }
    fread(destbuf, FI.psize, 1, file);
    fclose(file);
    //
	//printf("Create diff\n");
    createDiff(patchsize);
    //
	//printf("Create diff\n");
    file=fopen(currentpath, "wb");
    fwrite(destbuf, FI.psize, 1, file);
    
	printf("File %s patched\n", currentpath);
    fclose(file);
    ++current;
    return true;
}

//
void PatchClass::createDiff(unsigned long size)
{
    unsigned long ofs, len, count=0;
    while (count<size) {

   unsigned char size1[4];
memcpy(&size1,(srcbuf+count),4);
len=size1[3]*256*256*256+size1[2]*256*256+size1[1]*256+size1[0];

//        memcpy(&len, srcbuf+count, sizeof(len));
        count+=sizeof(len);

memcpy(&size1,(srcbuf+count),4);
ofs=size1[3]*256*256*256+size1[2]*256*256+size1[1]*256+size1[0];

//        memcpy(&ofs, srcbuf+count, sizeof(ofs));
        count+=sizeof(ofs);
        memcpy(destbuf+ofs, srcbuf+count, len);
        count+=len;
    }
}
