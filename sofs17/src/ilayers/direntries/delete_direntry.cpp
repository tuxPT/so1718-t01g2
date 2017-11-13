/**
 *  \author João Paulo Alpendre Aniceto 72255
 *  \tester João Paulo Alpendre Aniceto 72255
 */

#include "czdealer.h"
#include "itdealer.h"
#include "fileclusters.h"
#include "direntries.h"
#include "direntries.bin.h"

#include "probing.h"
#include "exception.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

uint32_t soDeleteDirEntry(int pih, const char *name, bool clean)
{

	//#define __original__
    #ifdef __original__
    	soProbe(353, "soDeleteDirEntry(%d, %s, %s)\n", pih, name, clean ? "true" : "false");
   		return soDeleteDirEntryBin(pih, name, clean);
	#else

	SOInode* Pinode=iGetPointer(pih);


	//inode pai não é diretorio
	if(!(Pinode->mode && S_IFDIR)){
		throw SOException(ENOENT,__FUNCTION__);
	}

    uint32_t IN=soGetDirEntry(pih,name);
	
    //nome do dirEntry demasiado pequeno ou vazio
    if(name[0]=='\0'||name==NULL){
    	throw SOException(EINVAL,__FUNCTION__);
    }

    //nome demasiado grande
    if(sizeof(name)>SOFS17_MAX_NAME){
    	throw SOException(ENAMETOOLONG,__FUNCTION__);
    }

    //dirEntry nao existe neste diretorio atual
	if(IN==NullReference){
		throw SOException(ENOENT,__FUNCTION__);
	}

	int cih=iOpen(IN);
	SOInode* inode=iGetPointer(cih);


	SODirEntry DirEntryList[DirentriesPerCluster];


	//percorre os clusters com dirEntries
	for(uint32_t j=0;j<inode->size/ClusterSize;j++){
		soReadFileCluster( cih, j, DirEntryList);

		//percorre as direntries de cada cluster
		for(uint32_t i=0;i<DirentriesPerCluster;i++){
			if(DirEntryList[i].name==name){
				if(clean){
					memset(DirEntryList[i].name,'\0',SOFS17_MAX_NAME);
				}
				else{
					DirEntryList[i].name[sizeof(DirEntryList[i].name)] = DirEntryList[i].name[0];
					DirEntryList[i].name[0]='\0';
				}
				uint32_t cn = soGetFileCluster(cih,j);
				soWriteCluster(cn,DirEntryList);
				iSave(cih);
				iClose(cih);
				return IN;
			}
		}
	}

	return NullReference;
    #endif
}
