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
#include <sys/stat.h>
#include <cmath>




static void soAllocIndirectFileCluster(SOInode * ip, uint32_t fcn, uint32_t * cnp);
static void soAllocDoubleIndirectFileCluster(SOInode * ip, uint32_t fcn, uint32_t * cnp);


/* ********************************************************* */

uint32_t soAllocFileCluster(int ih, uint32_t fcn)
{
	//#define __original__
    #ifdef __original__
    	soProbe(401, "soAllocFileCluster(%d, %u)\n", ih, fcn);
		return soAllocFileClusterBin(ih, fcn);
	#else
	

	iCheckHandler(ih,__FUNCTION__);
	SOInode* inode = iGetPointer(ih);
	uint32_t clusterNumber;

	if(fcn<N_DIRECT){
		clusterNumber = soAllocCluster();
		inode->clucnt++;
		inode->d[fcn]=clusterNumber;
	}
	else if(fcn-N_DIRECT<ReferencesPerCluster){
		soAllocIndirectFileCluster(inode, fcn-N_DIRECT, &clusterNumber);
	}
	else{

		soAllocDoubleIndirectFileCluster(inode, fcn-N_DIRECT-ReferencesPerCluster, &clusterNumber);
	}
	
	iSave(ih);
	return clusterNumber;
    #endif
}

//#if 0
/* ********************************************************* */

/* only a hint to decompose the solution */
static void soAllocIndirectFileCluster(SOInode * ip, uint32_t afcn, uint32_t * cnp)
{
    //soProbe(401, "soAllocIndirectFileCluster(%p, %u, %p)\n", ip, afcn, cnp);

	uint32_t cluster[ReferencesPerCluster];

	if(ip->i1==NullReference){
		SOSuperBlock* sb = sbGetPointer();			// Ponteiro para superblock
		
		if(sb->cfree>1){							// Verficar se existem pelo menos 2 inodes livre pq vai ser preciso allocar 1 extra para referencias
			ip->i1=soAllocCluster();
			ip->clucnt++;							// incrementar o nr de clusters usados do inode
			

			soReadCluster(ip->i1,&cluster);
			memset(cluster,NullReference,ReferencesPerCluster*sizeof(uint32_t));		//formatar cluster como lista de referencias
		}
		else{

			throw SOException(ENOSPC, __FUNCTION__);
		}								
	}
	else{

		soReadCluster(ip->i1,&cluster);					//ler as referencias do cluster i1
	}
	
	
	*cnp=soAllocCluster();							//guardar o nº de cluster					
	ip->clucnt++;
	
	cluster[afcn]=*cnp;				

	soWriteCluster(ip->i1,&cluster);				//guardar as referencias 

}

/* ********************************************************* */

/* only a hint to decompose the solution */
static void soAllocDoubleIndirectFileCluster(SOInode * ip, uint32_t afcn, uint32_t * cnp)
{
    //soProbe(401, "soAllocDoubleIndirectFileCluster(%p, %u, %p)\n", ip, afcn, cnp);
	
	uint32_t clusteri2[ReferencesPerCluster];
	uint32_t clusteri1[ReferencesPerCluster];
	uint32_t a=floor(afcn/ReferencesPerCluster); //clusteri2[a] da-nos o index do cluster i1
	uint32_t b=afcn%ReferencesPerCluster; 		 //i1[afcn] da-nos o index do cluster a alocar


	if(ip->i2==NullReference){
		SOSuperBlock* sb = sbGetPointer();			// Ponteiro para superblock
		
		if(sb->cfree>2){							// Verficar se existem pelo menos 3 inodes livre pq vai ser preciso allocar 2 extra para referencias
			ip->clucnt++;							// incrementar o nr de clusters usados do inode
			ip->i2=soAllocCluster();
			


			soReadCluster(ip->i2,clusteri2);		//formatar cluster como lista de referencias
			memset(clusteri2,NullReference,ReferencesPerCluster*sizeof(uint32_t));
		}
		else{

			throw SOException(ENOSPC, __FUNCTION__);
		}								
	}
	else{
		soReadCluster(ip->i2,clusteri2);
	}

	if(clusteri2[a]==NullReference){
		ip->clucnt++;	
		clusteri2[a]=soAllocCluster();
		soReadCluster(clusteri2[a],clusteri1);			//ler as referencias do cluster i2[pos]
		memset(clusteri1,NullReference,ReferencesPerCluster*sizeof(uint32_t));
	}
	else{
		soReadCluster(clusteri2[a],clusteri1);			//ler as referencias do cluster i2[pos]
	}


	*cnp=soAllocCluster();							//guardar o nº de cluster					
	ip->clucnt++;	
	clusteri1[b]=*cnp;				

	soWriteCluster(clusteri2[a],clusteri1);	
	
	soWriteCluster(ip->i2,clusteri2);			//guardar as referencias 
	
    
}
//#endif
