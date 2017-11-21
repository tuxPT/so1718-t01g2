/**
 *  \author Rodrigo Cunha e Silva
 *  \tester Rodrigo Cunha e Silva
 */

#include "direntries.h"
#include "direntries.bin.h"

#include "probing.h"
#include "exception.h"

#include <errno.h>

#include <czdealer.h>
#include <itdealer.h>
#include <fileclusters.h>

void soCheckEmptiness(int ih)
{
    soProbe(358, "soCheckEmptiness(%d)\n", ih);

    #ifdef __original__

    //soCheckEmptinessBin(ih);
    #else

    // Check handler and get pointer
    iCheckHandler(ih);
    SOInode * inode = iGetPointer(ih);

    SODirEntry dir_entry_list[DirentriesPerCluster];

    int i = 0;
    for (i = 0; i < inode->size; i++)
    {
    	// Read file cluster
    	soReadFileCluster(ih, i, dir_entry_list);

    	int start = 0;
    	if (i == 0 && strcmp(dir_entry_list[0].name, ".") == 0 && strcmp(dir_entry_list[1].name, "..") == 0)
    		start = 2;
    	
    	int j = start;
    	// // Iterate dir entries
    	for (j = start; j < DirentriesPerCluster; j++)
    	{
    		if (dir_entry_list[j].in != 0)
    		{
             	// Function return type should be boolean? NOTE: expected behaviour not specified in neither empty or not empty
                // Throw exception
                throw SOException(EINVAL,__FUNCTION__);
    			// return false;

    		}
    	}
    }

    #endif
}
