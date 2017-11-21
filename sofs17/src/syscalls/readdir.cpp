/*
 * \author Ricardo Ferreira Martins,72286
 * \tester Ricardo Ferreira Martins,72286
 */

#include "syscalls.h"
#include "syscalls.bin.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <time.h>
#include <utime.h>
#include <libgen.h>
#include <string.h>
 
#include "itdealer.h"
#include "direntry.h"
#include "datatypes.h"
#include "direntries.h"
#include "fileclusters.h"
#include <math.h>
#include "direntry.h"
 
#include "probing.h"
#include "exception.h"
 

int soReaddir(const char *path, void *buff, int32_t pos)
{
    soProbe(234, "soReaddir(\"%s\", %p, %u)\n", path, buff, pos);
 
    try
    {
 
 
        SODirEntry direntrieA[DirentriesPerCluster];
        SODirEntry direntrieB[DirentriesPerCluster];
       
        char *pathdup = strdupa(path);
        uint32_t inodeNumber = soTraversePath(pathdup);
       
        uint32_t inodeH = iOpen(inodeNumber);
        SOInode *inode = iGetPointer(inodeH);
 
        if((inode->mode && S_IFDIR) == 0)
        {
            iClose(inodeH);
            throw SOException(ENOTDIR, __FUNCTION__);
        }
 
 
       
        uint32_t  direntryNumber = pos/sizeof(SODirEntry);
       
        int clusterNumber = direntryNumber/DirentriesPerCluster;
 
        int aux=1;
 
        soReadFileCluster(inodeH,clusterNumber,direntrieA);
       
        soReadFileCluster(inodeH,0,direntrieB);
       
        uint32_t i=0;
 
        for(i=direntryNumber;i<inode->size/sizeof(SODirEntry);i++){
            if(i%DirentriesPerCluster==0){
                soReadFileCluster(inodeH,i/DirentriesPerCluster,direntrieB);
            }
            if(direntrieB[i].name[0]=='\0')
                {
                    continue;
                }else{
                    aux=i;
                    break;
                }
        }
 
 
        memcpy(buff, direntrieA[aux].name,SOFS17_MAX_NAME);
 
        iClose(inodeH);
        return (sizeof(SODirEntry)*(aux-direntryNumber+1));
   
    }
    catch(SOException & err)
    {
        return -err.en;
    }
 
   
}