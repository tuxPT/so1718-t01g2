/**
 *  \author João Paulo Alpendre Aniceto 72255
 *  \tester João Paulo Alpendre Aniceto 72255
 */

#include "freelists.h"
#include "freelists.bin.h"

#include "probing.h"
#include "exception.h"

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
	bool original = false;
	
    if(original){
	
        soProbe(502, "soAllocInode(%"PRIu32", %p)\n", type);
    	return soAllocInodeBin(type);
    }
	
    else{

		//type the inode type (it must represent either a file 0100000, or a directory 0040000, or a symbolic link 0120000)

	    /*soOpenITDealer();
	    soCloseITDealer();*/
	    int ih1,ih2,ih3;
	    uint32_t inodeNumber;

    	soOpenSBDealer();

    	SOSuperBlock* sb = sbGetPointer();			// Ponteiro para superblock
    	inodeNumber = sb->ihead;

	    ih1=iOpen(sb->ihead);						// handler para a cabeça da lista de inodes livres

	    iCheckHandler(ih1, "soAllocInode");			// verificar se o ih1 existe (se existe pelo menos um inode livre)

	    SOInode* inode = iGetPointer(ih1);			// ponteiro para o inode a allocar
    	
    	/* Alterar inode*/
	    inode->mode = inode->mode | type;

	    inode->owner = getuid();
	    inode->group = getgid();
	    inode->ctime = time(NULL);
	    inode->mtime = time(NULL);
	    inode->atime = time(NULL);

	    sb->ihead = inode->next;					// mudar o nr do ihead do superblock
	    sb->ifree --;								// decrementar o nº de inodes livres


	    ih2 = iOpen(inode->next);
	    ih3 = iOpen(inode->prev);

	    iCheckHandler(ih2, "soAllocInode");
	    iCheckHandler(ih3, "soAllocInode");

	    SOInode* inodeNewHead = iGetPointer(ih2);
	    SOInode* inodeTail = iGetPointer(ih3);
  		
  		inodeNewHead->prev = inode->prev;
  		inodeTail->next = inode->next;

	    iSave(ih1);
	    iSave(ih2);
	    iSave(ih3);
	    iClose(ih1);
		iClose(ih2);
	    iClose(ih3);


		sbSave();
	    soCloseSBDealer();

	    return inodeNumber;
	}
}
