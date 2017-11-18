/*
 * \author ...
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
 *  \brief Change the name or the location of a file in the directory hierarchy of the file system.
 *
 *  It tries to emulate <em>rename</em> system call.
 *
 *  \param path path to an existing file
 *  \param newPath new path to the same file in replacement of the old one
 *
 *  \return 0 on success; 
 *      -errno in case of error, being errno the system error that better represents the cause of failure
 */
int soRename(const char *path, const char *newPath)
{
    #ifdef __original__
    soProbe(227, "soRename(\"%s\", \"%s\")\n", path, newPath);
    #else

        char *Path = strdupa(path);                                 // path do diretorio
        char *dirPath = dirname(strdupa(path));                     // path do até ao diretorio (exclusive)
        char *dir = basename(strdupa(path));                        // nome do diretorio
        char *dir2=basename(strdupa(newPath));
        try
        {
            uint32_t cin = soTraversePath(Path);

            if(cin == NullReference){
                throw SOException(ENOENT, __FUNCTION__);
            }

         // Se existe, abre inode
 
        int cih = iOpen(cin);                                                                
        SOInode* inode = iGetPointer(cih); 

        // verifica se é diretorio
        if((inode->mode && S_IFDIR) == 0)                                
        {
            iClose(cih);
            throw SOException(ENOTDIR,__FUNCTION__);                    
        }        



        uint16_t permissions = R_OK | W_OK | X_OK;
        bool checkPerm = iCheckAccess(cih,permissions);

        // verifica se tem permissoes necessarias
        if(!checkPerm)                               
        {
            iClose(cih);
            throw SOException(EACCES,__FUNCTION__);                    
        }               
 
        // ver se diretorio esta vazio
        if(inode->lnkcnt != 2 || !checkPerm)
        {        
            iClose(cih);
            throw SOException(ENOTEMPTY,__FUNCTION__);              
        }
        // Finalmente, renomear
  
        // Diretorio pai
        uint32_t pin = soTraversePath(dirPath);

        // inode pai  
        int pih = iOpen(pin);  

        soRenameDirEntry(pih, dir, dir2);

        iSave(cih);
        iSave(pih);

        iClose(cih);
        iClose(pih);


        }
        catch(SOException & err)
        {
            return -err.en;
        }

        return 0;
    #endif
}
