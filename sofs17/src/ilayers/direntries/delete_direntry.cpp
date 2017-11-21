/**
 *  \author João Paulo Alpendre Aniceto 72255
 *  \tester João Paulo Alpendre Aniceto 72255
 */

#include "czdealer.h"
#include "itdealer.h"
#include "fileclusters.h"
#include "fileclusters.bin.h"
#include "direntries.h"
#include "direntries.bin.h"

#include "probing.h"
#include "exception.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include <iostream>
using namespace std;


uint32_t soDeleteDirEntry(int pih, const char *name, bool clean)
{

	//#define __original__
	soProbe(353, "soDeleteDirEntry(%d, %s, %s)\n", pih, name, clean ? "true" : "false");
    #ifdef __original__
   		return soDeleteDirEntryBin(pih, name, clean);
	#else

	iCheckHandler(pih,__FUNCTION__);

	SOInode* Pinode=iGetPointer(pih);

	//inode pai não é diretorio
	if(!(Pinode->mode & S_IFDIR)){
		throw SOException(ENOTDIR,__FUNCTION__);
	}

    //nome do dirEntry demasiado pequeno ou vazio
    if(name[0]=='\0'||name==NULL){
    	throw SOException(EINVAL,__FUNCTION__);
    }

    //nome demasiado grande
    if(sizeof(name)>SOFS17_MAX_NAME){
    	throw SOException(ENAMETOOLONG,__FUNCTION__);
    }


	SODirEntry DirEntryList[DirentriesPerCluster];


	//percorre os clusters com dirEntries
	for(uint32_t j=0;j<Pinode->size/ClusterSize;j++){

		//soReadFileClusterBin( pih, j, DirEntryList);
		soReadFileCluster( pih, j, DirEntryList);


		//percorre as direntries de cada cluster
		for(uint32_t i=0;i<DirentriesPerCluster;i++){
			if(strcmp(DirEntryList[i].name,name)==0){
				uint32_t IN = DirEntryList[i].in;
				if(clean){
					memset(DirEntryList[i].name,'\0',SOFS17_MAX_NAME);
					DirEntryList[i].in=NullReference;

				}
				else{
					DirEntryList[i].name[SOFS17_MAX_NAME] = DirEntryList[i].name[0];
					DirEntryList[i].name[0]='\0';
				}


				//oWriteFileClusterBin(pih,j,DirEntryList);
				soWriteFileCluster(pih,j,DirEntryList);

				return IN;
			}
		}
	
	}

	//dirEntry nao existe neste diretorio atual
	throw SOException(ENOENT,__FUNCTION__);

    #endif
}
