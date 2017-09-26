/*
 *  \file direntries.h
 *  \brief Set of operations to manage directories and directory entries
 *
 *  \author Artur Pereira 2008-2009, 2016-2017
 *  \author Miguel Oliveira e Silva 2009, 2017
 *  \author António Rui Borges - 2012-2015
 *
 *  \remarks In case an error occurs, every function throws an SOException
 */

#ifndef __SOFS17_DIRENTRIES_BIN__
#define __SOFS17_DIRENTRIES_BIN__

#include "datatypes.h"
#include "inttypes.h"

/* ************************************************** */
/* \defgroup direntries direntries 
 * @{ 
 */
/* ************************************************** */

/*
 *  \brief Get the inode associated to the given path
 *
 *  The directory hierarchy of the file system is traversed to find
 *  an entry whose name is the rightmost component of
 *  <tt>path</tt>.
 *  The path is supposed to be absolute and each component of <tt>path</tt>,
 *  with the exception of the rightmost one,
 *  should be a directory name or symbolic link name to a path.
 *
 *  The process that calls the operation must have execution
 *  (x) permission on all the components of the path with
 *  exception of the rightmost one.
 *
 *  \param path the path to be traversed
 *  \return the corresponding inode number
 */
uint32_t soTraversePathBin(char *path);

/* ************************************************** */

/*
 *  \brief Get the inode associated to the given name
 *
 *  The directory contents, seen as an array of directory entries,
 *  is parsed to find an entry whose name is <tt>name</tt>.
 *
 *  The <tt>name</tt> must also be a <em>base name</em> and not a <em>path</em>,
 *  that is, it can not contain the character '/'.
 *
 *  \param pih inode handler of the parent directory
 *  \param name the name entry to be searched for
 *  \return the corresponding inode number
 */
uint32_t soGetDirEntryBin(int pih, const char *name);

/* ************************************************** */

/*
 *  \brief Add a new entry to the parent directory.
 *
 *  A direntry is added connecting the parent inode to the child inode.
 *  The refcount of the child inode is not incremented by this function.
 *
 *  \param pih inode handler of the parent inode
 *  \param name name of the entry
 *  \param cin number of the child inode
 */
void soAddDirEntryBin(int pih, const char *name, uint32_t cin);

/* ************************************************** */

/*
 *  \brief Remove an entry from a parent directory.
 *
 *  A direntry associated from the given directory is deleted.
 *  The refcount of the child inode is not decremented by this function.
 *
 *  \param pih inode handler of the parent inode
 *  \param name name of the entry
 *  \param clean if true (different than zero) clean the corresponding dir entry, otherwise keep it dirty
 *  \return the inode number in the deleted entry
 */
uint32_t soDeleteDirEntryBin(int pih, const char *name, bool clean = false);

/* ************************************************** */

/*
 *  \brief Rename an entry of a directory.
 *
 *  A direntry associated from the given directory is renamed.
 *
 *  \param pih inode handler of the parent inode
 *  \param name current name of the entry
 *  \param newName new name for the entry
 */
void soRenameDirEntryBin(int pih, const char *name, const char *newName);

/* ************************************************** */
/* @} */
/* ************************************************** */

#endif             /* __SOFS17_DIRENTRIES_BIN__ */
