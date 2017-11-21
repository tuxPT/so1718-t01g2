/**
 *  \author Ricardo Ferreira Martins, 72286
 *  \tester Ricardo Ferreira Martins, 72286
 */

#include "direntries.h"
#include "direntries.bin.h"
#include "itdealer.h"
#include "probing.h"
#include "exception.h"

#include <errno.h>

void soRenameDirEntry(int pih, const char *name, const char *newName)
{
    soProbe(354, "soRenameDirEntry(%d, %s, %s)\n", pih, name, newName);
    //soRenameDirEntryBin(pih, name, newName);
        //nome do dirEntry demasiado pequeno ou vazio     
        SOInode* InodePai=iGetPointer(pih)
        if(!(inodePai->mode && S_IFDIR)){
             throw IOException(ENOENT,__FUNCTION__);
        }  
        if(newName[0]=='\0'||newName==NULL){
           throw SOException(EINVAL,__FUNCTION__);
        }
        // nome demasiado grande
        if(sizeof(newName)>SOFS17_MAX_NAME){
        throwSOExeception(ENAMETOOLONG,__FUNCTION__);
        }
	uint32_t cin = soGetDirEntry(pih,name);
	soDeleteDirEntry(pih,name,true);	
	soAddDirEntry(pih,newName,cin);
	
}
