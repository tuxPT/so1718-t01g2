/**
 *  \author Ricardo Ferreira Martins, 72286
 *  \tester Ricardo Ferreira Martins, 72286
 */

#include "fileclusters.h"
#include "fileclusters.bin.h"

#include "probing.h"
#include "exception.h"
#include "czdealer.h"
#include "datatypes.h"

#include <errno.h>
#include <inttypes.h>

void soWriteFileCluster(int ih, uint32_t fcn, void *buf)
{
    soProbe(405, "soWriteFileCluster(%d, %u, %p)\n", ih, fcn, buf);
 //   soWriteFileClusterBin(ih, fcn, buf);
	uint32_t cluster_num=soGetFileCluster(ih,fcn);
	if(cluster_num==NullReference)	cluster_num=soAllocFileCluster(ih,fcn);
	soWriteCluster(cluster_num,buf);
}
