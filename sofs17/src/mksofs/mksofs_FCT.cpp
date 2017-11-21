#include "mksofs.h"
#include "mksofs.bin.h"

#include "datatypes.h"
#include "rawdisk.h"
#include "exception.h"

#include <errno.h>

/* see mksofs.h for a description */
void fillInFreeClusterTable(uint32_t rmstart, uint32_t ctotal)
{
 	#ifdef __original__
   	 fillInFreeClusterTableBin(rmstart,ctotal);
    #else
		uint32_t blocos = ctotal / (8*ReferencesPerBitmapBlock);
		SORefBlock *refTable = new SORefBlock();
		refTable->cnt = (8*ReferenceBytesPerBitmapBlock)-1;
		refTable->idx = 0;
		for(uint32_t count = 0; count<=blocos; count+=1)
		{
			for(uint32_t i =0; i<=ReferencesPerBitmapBlock; i+=1)
			{
				refTable->map[i]=0xFF;

			}

			if(count==0)
			{
				refTable->map[0]=0x7F;
			}
			soWriteRawBlock(rmstart,refTable);
			rmstart+=BlockSize;
		}
	#endif
}
   