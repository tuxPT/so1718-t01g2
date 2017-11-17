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
		uint32_t node_handler = iOpen(in);
		iCheckHandler(node_handler);

		// Get pointer to node
		SOInode * node = iGetPointer(node_handler);

		// Apply bit mask
		node->mode = node->mode | INODE_FREE;

		// Read SB
		SOSuperBlock * sbp = sbGetPointer();

		// If free inode list is empty
		if (sbp->ifree == 0)
		{
			sbp->ihead = in;
			node->prev = in;
			node->next = in;
		}
		// If it has one element
		else if (sbp->ifree == 1)
		{
			// Get head node
			int head_handler = iOpen(sbp->ihead);
			SOInode * head_node = iGetPointer(head_handler);

			// Set head's previous and next node as input node
			head_node->prev = in;
			head_node->next = in;

			// Reciprocally, set previous and next of input node as head's number
			node->next = sbp->ihead;
			node->prev = sbp->ihead;

			// Save inodes
			iSave(head_handler);
			iSave(node_handler);
		}
		else
		{
			// Get head node
			int head_handler = iOpen(sbp->ihead);
			SOInode * head_node = iGetPointer(head_handler);

			// Get tail node

			int tail_handler = iOpen(head_node->prev);
			SOInode * tail_node = iGetPointer(tail_handler);

			// Set tail's next node as input node
			tail_node->next = in;

			// Set input node next as current head
			node->next = sbp->ihead;

			// Set previous as current tail
			node->prev = head_node->prev;

			// Set head's previous as input node
			head_node->prev = in;

			// TAIL <-> HEAD ===> TAIL <-> NEW <-> HEAD

			iSave(head_handler);
			iSave(tail_handler);
		}

		sbp->ifree++;

		sbSave();
		iSave(node_handler);
		iClose(node_handler);
#endif
}
