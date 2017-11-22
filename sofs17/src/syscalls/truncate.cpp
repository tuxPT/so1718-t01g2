/**
 *  \author Leonardo Miguel Oliveira Costa 80162
 *  \tester Leonardo Miguel Oliveira Costa 80162
 */

#include "syscalls.h"
#include "syscalls.bin.h"

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <time.h>
#include <utime.h>
#include <libgen.h>
#include <string.h>

#include "probing.h"
#include "exception.h"

//imports do aluno
#include "itdealer.h"
#include "czdealer.h"
#include "datatypes.h"
#include "direntries.h"
#include "inodeattr.h"
#include "fileclusters.h"
#include <cmath>


/*
 *  \brief Truncate a regular file to a specified length.
 *
 *  It tries to emulate <em>truncate</em> system call.
 *
 *  \param path path to the file
 *  \param length new size for the regular size
 *
 *  \return 0 on success; 
 *      -errno in case of error, being errno the system error that better represents the cause of failure
 */
int soTruncate(const char *path, off_t length)
{
    soProbe(231, "soTruncate(\"%s\", %u)\n", path, length);
    
    try
    {
        /* replace next line with your code */
        //soTruncateBin(path, length);
        if(length < 0 )
        {
            throw SOException(EINVAL, __FUNCTION__);
        }
        if(length > soGetMaxFileSize())
        {
            throw SOException(EFBIG, __FUNCTION__);
        }
        char* Path = strdupa(path);
        uint32_t in = soTraversePath(Path);
        int ih = iOpen(in);
        iCheckHandler(ih, __FUNCTION__);
        if(!iCheckAccess(ih, W_OK))
        {
            iSave(ih);
            iClose(ih);
            throw SOException(EACCES, __FUNCTION__);
        }
        SOInode* inode = iGetPointer(ih);
        if((inode->mode & S_IFREG) == 0 || (inode->mode & INODE_FREE) == 0)
        {
            iSave(ih);
            iClose(ih);
            throw SOException(ENOENT, __FUNCTION__);
        }
        if( inode->size > length)
        {
            soFreeFileClusters(ih, ceil(inode->size - length/float(ClusterSize))); 
        }
        inode->size = length;
        iSave(ih);
        iClose(ih);
    }
    catch(SOException & err)
    {
        return -err.en;
    }

    return 0;
}
