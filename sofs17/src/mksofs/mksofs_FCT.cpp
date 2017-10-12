#include "mksofs.h"
#include "mksofs.bin.h"

#include "datatypes.h"
#include "rawdisk.h"
#include "exception.h"

#include <errno.h>

/* see mksofs.h for a description */
void fillInFreeClusterTable(uint32_t rmstart, uint32_t ctotal)
{
	bool original = false;
	if(original)
	{
		fillInFreeClusterTableBin(rmstart,ctotal);
	}
	else
	{
		uint32_t rpc = (ReferencesPerBlock*BlocksPerCluster)-1;

		uint32_t clusterTable[rpc+1];

		uint32_t cfree = ctotal-1;

		uint32_t ccluster = rmstart+1;

		uint32_t ncluster = cfree;

		for(uint32_t i =1; i<=ctotal; i+=1)
		{
			clusterTable = new(clusterTable) uint32_t[rpc+1]();
			for(uint32_t j = 0; j<rpc ; j+=1)
			{
				clusterTable[j] = ncluster ? ccluster++ : NullReference;
				ncluster -= ncluster ? 1 : 0;
			}

			clusterTable[rpc]= ncluster ? (i+1) : NullReference;

			soWriteRawBlock(i*BlocksPerCluster+rmstart, clusterTable);

		}

	}
   
}