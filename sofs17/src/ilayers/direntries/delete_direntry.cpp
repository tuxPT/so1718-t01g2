/**
 *  \author João Paulo Alpendre Aniceto 72255
 *  \tester João Paulo Alpendre Aniceto 72255
 */

#include "direntries.h"
#include "direntries.bin.h"

#include "probing.h"
#include "exception.h"

#include <errno.h>

uint32_t soDeleteDirEntry(int pih, const char *name, bool clean)
{
    //soProbe(353, "soDeleteDirEntry(%d, %s, %s)\n", pih, name, clean ? "true" : "false");
    //return soDeleteDirEntryBin(pih, name, clean);

    uint32_t IN=soGetDirEntry(pih,name);
	if(sizeof(name)>SOFS17_MAX_NAME || sizeof(name)<0 || IN==NullReference){
		throw SOException(ENOENT,__FUNCTION__);
	}
	int ih=iOpen(IN);
	uint32_t clusternumber;

	SODirEntry DirEntryList[DirentriesPerCluster];


	soReadCluster(clusternumber,&DirEntryList);

	for(int i=0;i<DirentriesPerCluster;i++){
		if(DirEntryList[i].name==name){
			if(clean){
				DirEntryList[i].name=memset(DirEntryList[i].name,'\0',SOFS17_MAX_NAME);
			}
			else{
				char tmp = DirEntryList[i].name[0];
				DirEntryList[i].name[0]==DirEntryList[i].name[sizeof(DirEntryList[i].name)];
				DirEntryList[i].name[sizeof(DirEntryList[i].name)]==tmp;
			}
			soWriteCluster(clusternumber,&DirEntryList);
			iSave(ih);
			iClose(ih);
			return IN;
		}
	}


    
}
