/**
 *  \author João Paulo Alpendre Aniceto 72255
 *  \tester João Paulo Alpendre Aniceto 72255
 */

#include "fileclusters.h"
#include "fileclusters.bin.h"

#include "probing.h"
#include "exception.h"
#include "inode.h"
#include "datatypes.h"

#include "itdealer.h"
#include "sbdealer.h"
#include "czdealer.h"
#include "freelists.h"

#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <cmath>
//#if 0
static void soAllocIndirectFileCluster(uint32_t *i1,SOInode * ip, uint32_t fcn, uint32_t * cnp);
static void soAllocDoubleIndirectFileCluster(SOInode * ip, uint32_t fcn, uint32_t * cnp);
//#endif

/* ********************************************************* */

uint32_t soAllocFileCluster(int ih, uint32_t fcn)
{
	
    //soProbe(401, "soAllocFileCluster(%d, %u)\n", ih, fcn);
	//return soAllocFileClusterBin(ih, fcn);
	
	//Não é preciso abrir nem fechar inode?

	iCheckHandler(ih,__FUNCTION__);
	SOInode* inode = iGetPointer(ih);
	uint32_t clusterNumber;
	
	
	if(fcn<N_DIRECT){
		inode->clucnt++;
		clusterNumber = soAllocCluster();
		inode->d[fcn]=clusterNumber;
	}
	else if(fcn-N_DIRECT<ReferencesPerCluster){
		soAllocIndirectFileCluster(NULL,inode, fcn-N_DIRECT, &clusterNumber);
	}
	else{

		soAllocDoubleIndirectFileCluster(inode, fcn-N_DIRECT-ReferencesPerCluster, &clusterNumber);
	}

	iSave(ih);
	return clusterNumber;
    
}

//#if 0
/* ********************************************************* */

/* only a hint to decompose the solution */
static void soAllocIndirectFileCluster(uint32_t *i1, SOInode * ip, uint32_t afcn, uint32_t * cnp)
{
    //soProbe(401, "soAllocIndirectFileCluster(%p, %u, %p)\n", ip, afcn, cnp);
	if(i1 ==NULL){
		*i1= ip->i1;
	}

	uint32_t cluster[ReferencesPerCluster];

	if(ip->i1==NullReference){
		SOSuperBlock* sb = sbGetPointer();			// Ponteiro para superblock
		
		if(sb->cfree>2){							// Verficar se existem pelo menos 2 inodes livre pq vai ser preciso allocar 1 extra para referencias
			ip->clucnt++;						// incrementar o nr de clusters usados do inode
			*i1=soAllocCluster();
		}
		else{

			throw SOException(ENOSPC, __FUNCTION__);
		}								
	}

	soReadCluster(*i1,&cluster);					//ler as referencias do cluster i1
	ip->clucnt++;	
	*cnp=soAllocCluster();							//guardar o nº de cluster					

	cluster[afcn]=*cnp;				

	soWriteCluster(*i1,&cluster);				//guardar as referencias 

}

/* ********************************************************* */

/* only a hint to decompose the solution */
static void soAllocDoubleIndirectFileCluster(SOInode * ip, uint32_t afcn, uint32_t * cnp)
{
    //soProbe(401, "soAllocDoubleIndirectFileCluster(%p, %u, %p)\n", ip, afcn, cnp);
	
	uint32_t clusteri2[ReferencesPerCluster];
	//uint32_t clusterB[ReferencesPerCluster];
	uint32_t a=floor(afcn/ReferencesPerCluster); //clusteri2[a] da-nos o index do cluster i1
	uint32_t b=afcn%ReferencesPerCluster; 		 //i1[afcn] da-nos o index do cluster a alocar


	if(ip->i2==NullReference){
		SOSuperBlock* sb = sbGetPointer();			// Ponteiro para superblock
		
		if(sb->cfree>3){							// Verficar se existem pelo menos 3 inodes livre pq vai ser preciso allocar 2 extra para referencias
			ip->clucnt++;						// incrementar o nr de clusters usados do inode
			ip->i2=soAllocCluster();
		}
		else{

			throw SOException(ENOSPC, __FUNCTION__);
		}								
	}
	
	soReadCluster(ip->i2,&clusteri2);
	soAllocIndirectFileCluster(&clusteri2[a],ip,b,cnp);

	/*if(clusteri2[a]==NullReference){

		clusteri2[a]=soAllocCluster();
	}


	soReadCluster(clusterA[a],&clusterB);			//ler as referencias do cluster i2[pos]

	*cnp=soAllocCluster();							//guardar o nº de cluster					

	clusterB[b]=*cnp;				

	soWriteCluster(clusterA[a],&clusterB);	
	*/			
	soWriteCluster(ip->i2,&clusteri2);			//guardar as referencias 
	
    
}
//#endif
