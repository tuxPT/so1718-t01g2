/**
 *  \author João Paulo Alpendre Aniceto 72255
 *  \tester João Paulo Alpendre Aniceto 72255
 */

#include "freelists.h"
#include "freelists.bin.h"

#include "probing.h"
#include "exception.h"
#include "datatypes.h"
#include "sbdealer.h"
#include "itdealer.h"


#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
/*
 * Dictates to be obeyed by the implementation:
 * - error ENOSPC should be thrown if there is no free inodes
 * - the allocated inode must be properly initialized
 */

uint32_t soAllocInode(uint32_t type)
{

    //soProbe(502, "soAllocInode(%"PRIu32", %p)\n", type);
	//return soAllocInodeBin(type);

	
	//type the inode type (it must represent either a file 0100000, or a directory 0040000, or a symbolic link 0120000)

    
    int ih1,ih2,ih3,prev,next;
    uint32_t inodeNumber;


	SOSuperBlock* sb = sbGetPointer();			// Ponteiro para superblock

	//error ENOSPC should be thrown if there is no free inodes
	if(sb->ifree==0){
		throw SOException(ENOSPC, __FUNCTION__);
    }

	inodeNumber = sb->ihead;

    ih1=iOpen(sb->ihead);						// handler para a cabeça da lista de inodes livres

    iCheckHandler(ih1,__FUNCTION__);			// verificar se o ih1 existe (se existe pelo menos um inode livre)

    SOInode* inode = iGetPointer(ih1);			// ponteiro para o inode a allocar

    next=inode->next;
    prev=inode->prev;

	// Alterar inode
    inode->mode = uint16_t(type) ;

    inode->owner = getuid();
    inode->group = getgid();
    inode->atime = time(NULL);
    inode->mtime = time(NULL);
    inode->ctime = time(NULL);

    iSave(ih1);
    iClose(ih1);
    
    sb->ifree--;								// decrementar o nº de inodes livres
    if(sb->ifree>1){//caso haja 2 ou mais inodes livres 
    	sb->ihead = next;					// mudar o nr do ihead do superblock
    	ih2 = iOpen(next);
	    ih3 = iOpen(prev);

	    iCheckHandler(ih2,__FUNCTION__);
	    iCheckHandler(ih3,__FUNCTION__);

	    SOInode* inodeNewHead = iGetPointer(ih2);	
	    SOInode* inodeTail = iGetPointer(ih3);
  		
  		inodeNewHead->prev = prev;		//o novo ihead->prev vai apontar para a cauda da lista de inodes livres
  		inodeTail->next = next;			//o  inodetail->next vai apontar para a nova cabeça da lista de inodes livres

	    iSave(ih2);
	    iSave(ih3);
		iClose(ih2);
	    iClose(ih3);

    }
    else if(sb->ifree==1){//caso haja so 1 inode livre 
    	sb->ihead = next;					// mudar o nr do ihead do superblock

    	ih2 = iOpen(next);

	    iCheckHandler(ih2,__FUNCTION__);

	    SOInode* inodeNewHead = iGetPointer(ih2);
  		
  		inodeNewHead->prev = prev;		// a lista so vai ter 1 inode livre e vai apontar para si mesmo
  		inodeNewHead->next = next;		//
	   
	    iSave(ih2);
		iClose(ih2);

    }
    else{//se ja nao houver inodes livres
    	sb->ihead = NullReference;
    }



	sbSave();

    return inodeNumber;

	
}
