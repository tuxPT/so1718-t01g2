/**
 *  \author João Paulo Alpendre Aniceto 72255
 *  \tester João Paulo Alpendre Aniceto 72255
 */

#include "mksofs.h"
#include "mksofs.bin.h"

#include "datatypes.h"
#include "rawdisk.h"
#include "exception.h"

#include <errno.h>

#include <math.h>
#include <unistd.h>
#include <sys/types.h>

/* see mksofs.h for a description */
void fillInInodeTable(uint32_t itstart, uint32_t itsize)
{
	bool original = false;
    if(original){
    	fillInInodeTableBin(itstart, itsize);
   	}
    else{
	    unsigned int i,j,k=2;

	    uint32_t ittl=ceil(itsize*InodesPerBlock); //nº total inodes
	    
	    SOInode inodeTable[InodesPerBlock];
	    memset(inodeTable, 0, BlockSize);
	  

	    //creation of (root directory) = inode 0:
	    
	    inodeTable[0].mode = 0040000 | 0700 | 070 | 05; // 
	    inodeTable[0].lnkcnt = 2;
	    inodeTable[0].owner = getuid();
	    inodeTable[0].group = getgid();

	    
	    inodeTable[0].clucnt = ceil(float(sizeof(SODirEntry))/float(ClusterSize));//Nc = tamanho do ficheiro em bytes  / tamanha de um cluster em bytes
	    inodeTable[0].size = inodeTable[0].clucnt*4*BlockSize; 
	    


	    inodeTable[0].ctime = time(NULL);
	    inodeTable[0].mtime = time(NULL);
	    inodeTable[0].atime = time(NULL);

	    inodeTable[0].d[0] = 0; // primeiro data cluster

	    /*referencias para clusters a NULL*/
	    for (i = 1; i < N_DIRECT; i++) {
	        inodeTable[0].d[i] = NullReference;
	    }
	   
	    
	    inodeTable[0].i1 = NullReference;
	    inodeTable[0].i2 = NullReference;
	   	

        /*prencher os restantes inodes do primeiro bloco da tabela de inodes*/
	    for(i=1; i<InodesPerBlock;i++)
	    {
	        inodeTable[i].mode=INODE_FREE;

	        
	        inodeTable[i].prev = k-2;
	        inodeTable[i].next = k++; 

	        for (int k = 0; k < N_DIRECT; k++) {
	            inodeTable[i].d[k] = NullReference;
	        }

	        inodeTable[i].i1 = NullReference;
	        inodeTable[i].i2 = NullReference;
		        

		}		
	   
	    inodeTable[1].prev = uint32_t(ittl-1);
	    soWriteRawBlock(itstart, &inodeTable);
	    

	    /*Prencher o resto da tabela um bloco de cada vez*/
	      

	    for(j=1;j < itsize;j++)
	    {
	    	memset(inodeTable, 0, BlockSize);
	        for(i=0; i<InodesPerBlock;i++)
		    {
		        inodeTable[i].mode=INODE_FREE;

		        inodeTable[i].prev = uint32_t(k-2);
		        inodeTable[i].next = k+1>ittl ? uint32_t(1):uint32_t(k++); 

		        for (int k = 0; k < N_DIRECT; k++) {
		            inodeTable[i].d[k] = NullReference;
		        }

		        inodeTable[i].i1 = NullReference;
		        inodeTable[i].i2 = NullReference;
			        

			}
	        soWriteRawBlock(itstart+j, &inodeTable);
	    }
    }
}
