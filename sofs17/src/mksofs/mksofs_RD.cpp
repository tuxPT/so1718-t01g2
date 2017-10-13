#include "mksofs.h"
#include "mksofs.bin.h"

#include "datatypes.h"
#include "rawdisk.h"
#include "exception.h"

#include <errno.h>

/* see mksofs.h for a description */
void fillInRootDir(uint32_t rtstart)
{
	bool original = false;
	if (original)
	{
    	fillInRootDirBin(rtstart);
    }
    else
    {
    	int size = BlockSize * BlocksPerCluster / sizeof(SODirEntry); // 512 * 4 / 64
    	SODirEntry arr[size];
    	
    	// Fill default entries
    	arr[0].in = 0;
    	strncpy(arr[0].name, ".", sizeof(arr[0].name));

    	arr[1].in = 0;
    	strncpy(arr[1].name, "..", sizeof(arr[0].name));

    	// Fill the rest of the elements as null and zeros
    	int i;
    	for (i = 2;i < size; i++)
    	{
    		arr[i].in = NullReference;
    		memset(arr[i].name, 0, sizeof(arr[0].name));
    	}

    	// Write block by block
    	for (i = 0; i < 4; i++)
    	{	
    		soWriteRawBlock(rtstart + i, &arr[i*size/4]);
    	}
    }
}
