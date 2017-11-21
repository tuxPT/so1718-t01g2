/**
 *  \author Ariel António da Silva Bastos, 72204
 *  \tester ...
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


uint32_t soGetDirEntry(int pih, const char *name)
{
   
    #ifdef __original__
    	 soProbe(351, "soGetDirEntry(%d, %s)\n", pih, name);
   		return soGetDirEntryBin(pih, name);
	#else
    SOInode *Pinode = iGetPointer(pih);
    
    //inode pai não é diretorio
    if(!(Pinode->mode && S_IFDIR)){
		throw SOException(ENOENT,__FUNCTION__);
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

	for(uint32_t j=0;j<Pinode->size/ClusterSize;j++){

		//soReadFileClusterBin( pih, j, DirEntryList);
		soReadFileCluster( pih, j, DirEntryList);


		//percorre as direntries de cada cluster
		for(uint32_t i=0;i<DirentriesPerCluster;i++){
			if(strcmp(DirEntryList[i].name,name)==0){
				uint32_t IN = DirEntryList[i].in;
				return IN;
			}
		}
	
	}

	throw SOException(ENOENT,__FUNCTION__);
	return NullReference;
    #endif
}
