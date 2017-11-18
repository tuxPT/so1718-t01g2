/*
 * \author Rodrigo Cunha e Silva
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

#include "czdealer.h"
#include "itdealer.h"
#include "inodeattr.h"
#include "direntries.h"
#include "freelists.h"
#include "fileclusters.h"

/*
 *  \brief Create a directory.
 *
 *  It tries to emulate <em>mkdir</em> system call.
 *
 *  \param path path to the file
 *  \param mode permissions to be set:
 *          a bitwise combination of S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP, S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH
 *
 *  \return 0 on success; 
 *      -errno in case of error, being errno the system error that better represents the cause of failute
 */
int soMkdir(const char *path, mode_t mode)
{
    soProbe(232, "soMkdir(\"%s\", %u)\n", path, mode);

    try
    {
        /* replace next line with your code */
        // soMkdirBin(path, mode);

        char * xpath = strdupa(path);
        char * bn = strdupa(basename(xpath));
        char * dn = dirname(xpath);
    
        // // Check lengths
        // if (strlen(xpath)>??MAX PATH?)
        //     throw SOException(ENAMETOOLONG,__FUNCTION__);
 
        if (strlen(bn)>SOFS17_MAX_NAME)
            throw SOException(ENAMETOOLONG,__FUNCTION__);
        
        // Is there a path to the target dir?
        unsigned int parent_inumber = soTraversePath(dn);
    
        if(parent_inumber == NullReference)
            throw SOException(ENOENT,__FUNCTION__);

        // Get inode handler to 
        uint32_t pih = iOpen(parent_inumber);
        SOInode * pi = iGetPointer(pih);

        // If paren't isn't dir, abort
        if ((pi->mode & S_IFDIR) != S_IFDIR)
        {
            iSave(pih);
            iClose(pih);
            throw SOException(ENOTDIR,__FUNCTION__);
        }

        // Check write access
        if(iCheckAccess(pih, W_OK)==false){
            iSave(pih);
            iClose(pih);
            throw SOException(EACCES,__FUNCTION__);
        }

        // Check if target dir already exists
        uint32_t dir = soGetDirEntry(pih, bn);

        if(dir!=NullReference){
            iSave(pih);
            iClose(pih);
            throw SOException(EEXIST, __FUNCTION__);
        }

        // Allocate inode
        uint32_t child_inumber = soAllocInode(S_IFDIR);
        uint32_t cih = iOpen(child_inumber);

        // If no inode available, abort
        if (cih==NullReference)
        {
            iSave(pih);
            iClose(pih);
            iSave(cih);
            iClose(cih);
            throw SOException(ENOMEM,__FUNCTION__);
        }
        
        // Otherwise update its mode
        iSetAccess(cih, mode);

        // Get node pointer
        SOInode * cin = iGetPointer(cih);
        SOInode * pin = iGetPointer(pih);

        // Add default entries to child node
        soAddDirEntry(cih, ".", child_inumber);
        cin->lnkcnt++;

        soAddDirEntry(cih, "..", parent_inumber);
        cin->lnkcnt++;
 
        // Add child to parent
        soAddDirEntry(pih, bn, child_inumber);
        pin->lnkcnt++;
        
        // Save and close-up 
        iSave(cih);
        iClose(cih);
        iSave(pih);
        iClose(pih);

    }
    catch(SOException & err)
    {
        return -err.en;
    }

    return 0;
}
