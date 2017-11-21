/*
 *  \author João Paulo Alpendre Aniceto 72255
 *  \tester João Paulo Alpendre Aniceto 72255
 */

#include "syscalls.h"
#include "syscalls.bin.h"

#include "czdealer.h"
#include "itdealer.h"
#include "inodeattr.h"
#include "direntries.h"
#include "direntries.bin.h"
#include "freelists.h"
#include "freelists.bin.h"
#include "fileclusters.h"
#include "fileclusters.bin.h"

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

/*
 *  \brief Delete a directory.
 *
 *  It tries to emulate <em>rmdir</em> system call.
 *  
 *  The directory should be empty, ie. only containing the '.' and '..' entries.
 *
 *  \param path path to the directory to be deleted
 *
 *  \return 0 on success; 
 *      -errno in case of error, being errno the system error that better represents the cause of failure
 */
int soRmdir(const char *path)
{
    //#define __original__
    soProbe(233, "soRmdir(\"%s\")\n", path);
    #ifdef __original__
        soRmdirBin(path);
    #else



    char *Path = strdupa(path);                                 // path do diretorio
    char *dirPath = dirname(strdupa(path));                     // path do até ao diretorio (exclusive)
    char *dir = basename(strdupa(path));                        // nome do diretorio

    try
    {

        // Ver se o caminho existe
        //uint32_t cin = soTraversePathBin(Path);
        uint32_t cin = soTraversePath(Path);                         


        //Existe?
        if(cin == NullReference){                                     
            throw SOException(ENOENT,__FUNCTION__);                                              
        } 
    
        

        // Se existe, abre inode
 
        int cih = iOpen(cin);                                                                
        SOInode* inode = iGetPointer(cih);
  
        
        // verifica se é diretorio
        if((inode->mode & S_IFDIR) == 0)                                
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

        //verifica se está vazio
        soCheckEmptiness(cih); 
 
        // Finalmente, remover diretorio
  
        // Diretorio pai
        //uint32_t pin = soTraversePathBin(dirPath);
        uint32_t pin = soTraversePath(dirPath);

        // inode pai  
        int pih = iOpen(pin);                                              

        
                                    

        //remove o direntry, do diretorio que vamos remover, do inode pai 
        soDeleteDirEntry(pih, dir, true);                
        //decrementa o linkcount do inode pai
        iDecLnkcnt(pih);

        //remove o direntry . do diretorio que vamos remover
        soDeleteDirEntry(cih, ".", false);   
        //remove o direntry .. do diretorio que vamos remover
        soDeleteDirEntry(cih, "..", false);   
        
        //reseta o linkcount do inode
        inode->lnkcnt=0;
    
        //Liberta todos os clusters do diretorio que vamos apagar
        //soFreeFileClustersBin(cih,0);
        soFreeFileClustersBin(cih,0);  
        

        //libertar Inode
        //soFreeInodeBin(cin);
        soFreeInodeBin(cin);

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
