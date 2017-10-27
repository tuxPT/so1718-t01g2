/**
 *  \author Rodrigo Cunha e Silva
 *  \tester Rodrigo Cunha e Silva
 */

#include "freelists.h"
#include "freelists.bin.h"

#include "probing.h"
#include "exception.h"
#include "datatypes.h"
#include "rawdisk.h"
#include "itdealer.h"
#include "sbdealer.h"

#include <errno.h>
#include <inttypes.h>

/*
 * Dictates to be obeyed by the implementation:
 * - parameter in must be validated, 
 *      throwing a proper error if necessary
 */
void soFreeInode(uint32_t in)
{
// #define __original__
#ifdef __original__
	soProbe(532, "soFreeInode(%" PRIu32 ")\n", in);
	soFreeInodeBin(in);
#else
		soOpenSBDealer();
		// Read i-node
		uint32_t ih = iOpen(in);
		iCheckHandler(ih);

		SOInode * node = iGetPointer(ih);

		// Apply bit mask
		node->mode = node->mode | INODE_FREE;

		// Read SB
		SOSuperBlock * sb = sbGetPointer();

		// Get head node
		int hhead = iOpen(sb->ihead);
		SOInode * headNode = iGetPointer(hhead);

		if (sb->ifree == 0)
		{
			sb->ihead = in;
			node->prev = in;
			node->next = in;
		}
		else if (sb->ifree == 1)
		{
			headNode->prev = in;
			headNode->next = in;

			node->next = sb->ihead;
			node->prev = sb->ihead;
			iSave(hhead);
		}
		else
		{
			int hprev = iOpen(headNode->prev);
			SOInode * previousNode = iGetPointer(hprev);
			previousNode->next = in;
			node->prev = headNode->prev;
			headNode->prev = in;
			node->next = sb->ihead;

			iSave(hhead);
			iSave(hprev);
		}

		sb->ifree++;

		iSave(hhead);
		soCloseSBDealer();
		iClose(ih);
#endif
}
