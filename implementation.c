/*
  MyFS: a tiny file-system written for educational purposes
  MyFS is
  Copyright 2018 by
  University of Alaska Anchorage, College of Engineering.
  Contributors: Christoph Lauter
                ...
                ... and
                ...
  and based on
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.
  gcc -Wall myfs.c implementation.c `pkg-config fuse --cflags --libs` -o myfs
*/

#include <stddef.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

#include "fsRecord.h"

#define TRUE 1
#define FALSE 0
#define FAILURE -1
#define BLOCKSIZE 512


/* The filesystem you implement must support all the 13 operations
   stubbed out below. There need no be support for access rights,
   links, symbolic links. There needs to be support for access and
   modification times and information for statfs.
   The filesystem must run in memory, using the memory of size
   fssize pointed to by fsptr. The memory comes from mmap and
   is backed with a file if a backup-file is indicated. When
   the filesystem is unmounted, the memory is written back to
   that backup-file. When the filesystem is mounted again from
   the backup-file, the same memory appears at the newly mapped
   in virtual address. The filesystem datastructures hence must not
   store any pointer directly to the memory pointed to by fsptr; it
   must rather store offsets from the beginning of the memory region.
   When a filesystem is mounted for the first time, the whole memory
   region of size fssize pointed to by fsptr reads as zero-bytes. When
   a backup-file is used and the filesystem is mounted again, certain
   parts of the memory, which have previously been written, may read
   as non-zero bytes. The size of the memory region is at least 2048
   bytes.
   CAUTION:
   * You MUST NOT use any global variables in your program for reasons
   due to the way FUSE is designed.
   You can find ways to store a structure containing all "global" data
   at the start of the memory region representing the filesystem.
   * You MUST NOT store (the value of) pointers into the memory region
   that represents the filesystem. Pointers are virtual memory
   addresses and these addresses are ephemeral. Everything will seem
   okay UNTIL you remount the filesystem again.
   You may store offsets/indices (of type size_t) into the
   filesystem. These offsets/indices are like pointers: instead of
   storing the pointer, you store how far it is away from the start of
   the memory region. You may want to define a type for your offsets
   and to write two functions that can convert from pointers to
   offsets and vice versa.
   * You may use any function out of libc for your filesystem,
   including (but not limited to) malloc, calloc, free, strdup,
   strlen, strncpy, strchr, strrchr, memset, memcpy. However, your
   filesystem MUST NOT depend on memory outside of the filesystem
   memory region. Only this part of the virtual memory address space
   gets saved into the backup-file. As a matter of course, your FUSE
   process, which implements the filesystem, MUST NOT leak memory: be
   careful in particular not to leak tiny amounts of memory that
   accumulate over time. In a working setup, a FUSE process is
   supposed to run for a long time!
   It is possible to check for memory leaks by running the FUSE
   process inside valgrind:
   valgrind --leak-check=full ./myfs --backupfile=test.myfs ~/fuse-mnt/ -f
   However, the analysis of the leak indications displayed by valgrind
   is difficult as libfuse contains some small memory leaks (which do
   not accumulate over time). We cannot (easily) fix these memory
   leaks inside libfuse.
   * Avoid putting debug messages into the code. You may use fprintf
   for debugging purposes but they should all go away in the final
   version of the code. Using gdb is more professional, though.
   * You MUST NOT fail with exit(1) in case of an error. All the
   functions you have to implement have ways to indicated failure
   cases. Use these, mapping your internal errors intelligently onto
   the POSIX error conditions.
   * And of course: your code MUST NOT SEGFAULT!
   It is reasonable to proceed in the following order:
   (1)   Design and implement a mechanism that initializes a filesystem
         whenever the memory space is fresh. That mechanism can be
         implemented in the form of a filesystem handle into which the
         filesystem raw memory pointer and sizes are translated.
         Check that the filesystem does not get reinitialized at mount
         time if you initialized it once and unmounted it but that all
         pieces of information (in the handle) get read back correctly
         from the backup-file.
   (2)   Design and implement functions to find and allocate free memory
         regions inside the filesystem memory space. There need to be
         functions to free these regions again, too. Any "global" variable
         goes into the handle structure the mechanism designed at step (1)
         provides.
   (3)   Carefully design a data structure able to represent all the
         pieces of information that are needed for files and
         (sub-)directories.  You need to store the location of the
         root directory in a "global" variable that, again, goes into the
         handle designed at step (1).
   (4)   Write __myfs_getattr_implem and debug it thoroughly, as best as
         you can with a filesystem that is reduced to one
         function. Writing this function will make you write helper
         functions to traverse paths, following the appropriate
         subdirectories inside the file system. Strive for modularity for
         these filesystem traversal functions.
   (5)   Design and implement __myfs_readdir_implem. You cannot test it
         besides by listing your root directory with ls -la and looking
         at the date of last access/modification of the directory (.).
         Be sure to understand the signature of that function and use
         caution not to provoke segfaults nor to leak memory.
   (6)   Design and implement __myfs_mknod_implem. You can now touch files
         with
         touch foo
         and check that they start to exist (with the appropriate
         access/modification times) with ls -la.
   (7)   Design and implement __myfs_mkdir_implem. Test as above.
   (8)   Design and implement __myfs_truncate_implem. You can now
         create files filled with zeros:
         truncate -s 1024 foo
   (9)   Design and implement __myfs_statfs_implem. Test by running
         df before and after the truncation of a file to various lengths.
         The free "disk" space must change accordingly.
   (10)  Design, implement and test __myfs_utimens_implem. You can now
         touch files at different dates (in the past, in the future).
   (11)  Design and implement __myfs_open_implem. The function can
         only be tested once __myfs_read_implem and __myfs_write_implem are
         implemented.
   (12)  Design, implement and test __myfs_read_implem and
         __myfs_write_implem. You can now write to files and read the data
         back:
         echo "Hello world" > foo
         echo "Hallo ihr da" >> foo
         cat foo
         Be sure to test the case when you unmount and remount the
         filesystem: the files must still be there, contain the same
         information and have the same access and/or modification
         times.
   (13)  Design, implement and test __myfs_unlink_implem. You can now
         remove files.
   (14)  Design, implement and test __myfs_unlink_implem. You can now
         remove directories.
   (15)  Design, implement and test __myfs_rename_implem. This function
         is extremely complicated to implement. Be sure to cover all
         cases that are documented in man 2 rename. The case when the
         new path exists already is really hard to implement. Be sure to
         never leave the filessystem in a bad state! Test thoroughly
         using mv on (filled and empty) directories and files onto
         inexistant and already existing directories and files.
   (16)  Design, implement and test any function that your instructor
         might have left out from this list. There are 13 functions
         __myfs_XXX_implem you have to write.
   (17)  Go over all functions again, testing them one-by-one, trying
         to exercise all special conditions (error conditions): set
         breakpoints in gdb and use a sequence of bash commands inside
         your mounted filesystem to trigger these special cases. Be
         sure to cover all funny cases that arise when the filesystem
         is full but files are supposed to get written to or truncated
         to longer length. There must not be any segfault; the user
         space program using your filesystem just has to report an
         error. Also be sure to unmount and remount your filesystem,
         in order to be sure that it contents do not change by
         unmounting and remounting. Try to mount two of your
         filesystems at different places and copy and move (rename!)
         (heavy) files (your favorite movie or song, an image of a cat
         etc.) from one mount-point to the other. None of the two FUSE
         processes must provoke errors. Find ways to test the case
         when files have holes as the process that wrote them seeked
         beyond the end of the file several times. Your filesystem must
         support these operations at least by making the holes explicit
         zeros (use dd to test this aspect).
   (18)  Run some heavy testing: copy your favorite movie into your
         filesystem and try to watch it out of the filesystem.
*/

/* OUR FUNCTIONS */

char isDirectory_fsRecord(void *fsptr, const char *path)
{
    //TODO: see if the last character is a slash or whatever
    //returning 0 right now (meaning it's not a directory), but need to add some metrics to check.
    return '0';
}

void debug_ustar(const char *statement) //Literally just a print statement.
{
    printf("%s\n", statement);
}

size_t calculateNumBlocksInFile(size_t size)
{
    size_t blocks = 1;  //All files have at least one block
    if(size <= PADDING_SIZE) return blocks; //Only one in this case

    size -= PADDING_SIZE;
    float extraBlocks = (float) size / (float) BLOCKSIZE;
    blocks += (size_t) extraBlocks; //truncs fractional blocks, take care of that next
    if(extraBlocks > (size_t) extraBlocks) blocks++;
    return blocks;
}

int findNextFreeBlock(void *fsptr, size_t fssize)
{
    struct fsRecord_header *h = (struct fsRecord_header *) fsptr;

    // TODO: read in size of file (which may take up multiple blocks), and skip those blocks if it takes up more than one
    // if a file contains "FIRE" it could kill this.

	printf("findNextFreeBlock( %p, %lu) called\n", fsptr, fssize);
    for(int i = 1; fssize > (i*BLOCKSIZE); i++)
    {
		// printf("loop\n");
        if(memcmp(&(h[i-1].eyecatch), "FIRES", 6) != 0)
        {
            printf("findNextFreeBlock: found free block at: %i\n", i-1);
            return i-1;
        }
    }
    printf("next free block not found.");
    return -1;
}

// Searches whole filesystem for a file with fName value equal to 'path'
int findFileBlock(void *fsptr, size_t fssize, const char *path){
	printf("findFileBlock( %p, %lu, %s ) called\n", fsptr, fssize, path);

    struct fsRecord_header *h = (struct fsRecord_header *) fsptr;

	for(int i = 1; fssize > (i*BLOCKSIZE); i++)
    {
        if(memcmp(&(h[i-1].eyecatch), "FIRES", 6) == 0)
        {
			if(!strncmp(h[i-1].fName, path, sizeof(h->fName))){
				printf("findFileBlock: found file: %s at block %i\n", path, i-1);
            	return i-1;
			}

        }
    }
	return FAILURE;
}



int init_fsRecord(void *fsptr, size_t fsSize, const char *path, char argIsDirectory, size_t fileSize)
{
    debug_ustar("init_fsRecord called");
    struct fsRecord_header *h = (struct fsRecord_header *) fsptr;

	// incBlock is offset from fsptr where file of size inFsSize can fit
    int incBlock = findNextFreeBlock(fsptr, fsSize);
	if(incBlock == -1){
		debug_ustar("findNextFreeBlock failed.");
		return FAILURE;
	}

    //strip "antisocial prefixes" here
    if(strlen(path) > (sizeof(h->fName)- 1))
    {
        debug_ustar("File name is too long.");
        return FAILURE;
    }
    //TODO: Make sure to read n-1 when reading through the buffer, due to null terminators being placed by strncpy


	printf("Initializing header for %s at block: %i\n",path, incBlock);

	strncpy(h[incBlock].eyecatch, "FIRES", sizeof(h->eyecatch));

    strncpy(h[incBlock].fName, path, sizeof h->fName);
    strncpy(h[incBlock].uid, "user0000", sizeof h->uid);

    strncpy(h[incBlock].gid, "group4b0", sizeof h->gid);
    snprintf(h[incBlock].fsSize, sizeof(h->fsSize), "%lu", fileSize);
    snprintf(h[incBlock].mtime, sizeof(h->mtime), "%lu", time(NULL));

    //The "check if directory" function (which needs to be written)
    //should be called before this function.

    h[incBlock].isDirectory = argIsDirectory;

    //TODO (maybe): command for finding group and usernames
    strncpy(h[incBlock].gname, "root", sizeof(h->gname));
    strncpy(h[incBlock].uname, "root", sizeof(h->uname));

    return TRUE;
}

// ONLY checks for an initialized block at fsptr
// Do we need to call on every single function? -- Yes. Otherwise we'll never know if it's actually initialized.
int checkFsInit(void *fsptr, size_t fssize, const char* path)
{
    if(!(fsptr)) //die
    {
        debug_ustar("checkFSInit FAILED: Null pointer has been passed.");
        return FAILURE;
    }

    struct fsRecord_header *h = (struct fsRecord_header *) fsptr;

    if(memcmp(h, "FIRES", 6) != 0) //file system has not been initialized
    {
        debug_ustar("checkFSInit: INITIALIZING FILESYSTEM");
        strncpy((char *)h, "FIRES", 6);
		int dot = init_fsRecord(fsptr, fssize, "/.", 't', 0);				// What do we call root directory?
		int dot_dot = init_fsRecord(fsptr, fssize, "/..", 't', 0);
		int slsh = init_fsRecord(fsptr, fssize, "/", 't', 0);

		if(dot && dot_dot && slsh){
			return TRUE;
		}
    }
	else{
		return TRUE;
	}
}

/* OUR FUNCTIONS */



/* Implements an emulation of the stat system call on the filesystem
   of size fssize pointed to by fsptr.
   If path can be followed and describes a file or directory
   that exists and is accessable, the access information is
   put into stbuf.
   On success, 0 is returned. On failure, -1 is returned and
   the appropriate error code is put into *errnoptr.
   man 2 stat documents all possible error codes and gives more detail
   on what fields of stbuf need to be filled in. Essentially, only the
   following fields need to be supported:
   st_uid      the value passed in argument
   st_gid      the value passed in argument
   st_mode     (as fixed values S_IFDIR | 0755 for directories,
                                S_IFREG | 0755 for files)
   st_nlink    (as many as there are subdirectories (not files) for directories
                (including . and ..),
                1 for files)
   st_size     (supported only for files, where it is the real file size)
   st_atim
   st_mtim
   struct stat {          Actual 'man 2 stat' definition
    dev_t     st_dev;     ID of device containing file
    ino_t     st_ino;     inode number
    mode_t    st_mode;    protection
    nlink_t   st_nlink;   number of hard links
    uid_t     st_uid;     user ID of owner
    gid_t     st_gid;     group ID of owner
    dev_t     st_rdev;    device ID (if special file)
    off_t     st_size;    total size, in bytes
    blksize_t st_blksize; blocksize for file system I/O
    blkcnt_t  st_blocks;  number of 512B blocks allocated
    time_t    st_atime;   time of last access
    time_t    st_mtime;   time of last modification
    time_t    st_ctime;   time of last status change
};
*/
int __myfs_getattr_implem(void *fsptr, size_t fssize, int *errnoptr,uid_t uid, gid_t gid,
const char *path, struct stat *stbuf)
    {
    	// printf("__myfs_getattr_implem( %p, %lu, %i, %i, %i, %s)\n", fsptr, fssize, *errnoptr, uid, gid, path);

        if(!checkFsInit(fsptr, fssize, path)){
			printf("Error: Filesystem cannot be initialized");
			return -1;
		}


        const struct fsRecord_header *h = (const struct fsRecord_header *) fsptr;
		int offset = findFileBlock(fsptr, fssize, path);

		if(offset == FAILURE){
			printf("Get attribute -- error: %s not found.\n", path);
			*errnoptr = ENOENT;
			return -1;
		}

		__mode_t mode = 0100000;			// Is file
		if(h[offset].isDirectory == 't'){
			mode = 0040000;					// Is directory
		}

        //What is "size_ul", not sure we need it.
        //Check for file name string length

		stbuf->st_uid = (__uid_t) h[offset].uid;
		stbuf->st_gid = (__gid_t) h[offset].gid;
		stbuf->st_mode = mode;
		//stbuf->st_nlink = NULL;
        stbuf->st_size = (__off_t) atol(h[offset].fsSize);
      	stbuf->st_atime = (time_t) h[offset].atime;
      	stbuf->st_mtime = (time_t) h[offset].mtime;

        // printf("__myfs_getattr_implem - SUCCESS\n");
        return 0;
}

/* Implements an emulation of the readdir system call on the filesystem
   of size fssize pointed to by fsptr.
   If path can be followed and describes a directory that exists and
   is accessable, the names of the subdirectories and files
   contained in that directory are output into *namesptr. The . and ..
   directories must not be included in that listing.
   If it needs to output file and subdirectory names, the function
   starts by allocating (with calloc) an array of pointers to
   characters of the right size (n entries for n names). Sets
   *namesptr to that pointer. It then goes over all entries
   in that array and allocates, for each of them an array of
   characters of the right size (to hold the i-th name, together
   with the appropriate '\0' terminator). It puts the pointer
   into that i-th array entry and fills the allocated array
   of characters with the appropriate name. The calling function
   will call free on each of the entries of *namesptr and
   on *namesptr.
   The function returns the number of names that have been
   put into namesptr.
   If no name needs to be reported because the directory does
   not contain any file or subdirectory besides . and .., 0 is
   returned and no allocation takes place.
   On failure, -1 is returned and the *errnoptr is set to
   the appropriate error code.
   The error codes are documented in man 2 readdir.
   In the case memory allocation with malloc/calloc fails, failure is
   indicated by returning -1 and setting *errnoptr to EINVAL.
*/
int __myfs_readdir_implem(void *fsptr, size_t fssize, int *errnoptr,
                          const char *path, char ***namesptr) {

    // printf("__myfs_readdir_implem( %p, %lu, %i, %s) \n", fsptr, fssize, *errnoptr, path);
    if(!checkFsInit(fsptr, fssize, path)){
			printf("Error: Filesystem cannot be initialized");
			return -1;
	}

	struct fsRecord_header *h = (struct fsRecord_header *) fsptr;
	char **names = malloc(NULL);
	size_t size_of_fName = 0;
	int names_count = 0;

	// iterate through FS blocks.

	for( int i = 1; fssize > (i*BLOCKSIZE); i++){
		if(memcmp(&(h[i-1].eyecatch), "FIRES", 6) == 0)
        {
			if(!strncmp(h[i-1].fName, path, strlen(path))){
				if(!strncmp(h[i-1].fName, "/.", strlen("/.")) || !strncmp(h[i-1].fName, "/..", strlen("/.."))){
					//printf("__myfs_readdir_implem: NOT INCLUDING directory: %s at block %i\n", h[i-1].fName, i-1);
					continue;
				}
				printf("__myfs_readdir_implem: found directory: %s at block %i\n", path, i-1);
				//printf("strlen(fName) = %lu, sizeof(fName) - %lu", strlen(h[i-1].fName), sizeof(h[i-1].fName) );
				names_count++;												// Iterate number of strings in list
				size_of_fName = (strlen(h[i-1].fName))+1;						// Get length of fName string
				char *fName_str = malloc(size_of_fName * sizeof(char));		// Allocate memory for fName string
				strncpy(fName_str, h[i-1].fName, size_of_fName);			// Copy value of fName into mem-allocated pointer
				names = realloc(names, (sizeof(void *))*names_count);		// Reallocate memory for pointer list
				names[names_count-1] = fName_str;							// Set fName ptr to names_count-1 because of ++ at beginning
			}
        }
	}

	printf("__myfs_readdir_implem: NAMES LIST: ");
	for (int i = 0; i < names_count; i++){
		printf("%s  ", (char *)names[i]);
	}
	printf("\n");

	if(names == NULL){
		printf("__myfs_readdir_implem: no files or directories found.");
		return 0;
	}

	*namesptr = names;


	// printf("__myfs_readdir_implem - SUCCESS\n");
    return names_count;
}

/* Implements an emulation of the mknod system call for regular files
   on the filesystem of size fssize pointed to by fsptr.
   This function is called only for the creation of regular files.
   If a file gets created, it is of size zero and has default
   ownership and mode bits.
   The call creates the file indicated by path.
   On success, 0 is returned.
   On failure, -1 is returned and *errnoptr is set appropriately.
   The error codes are documented in man 2 mknod.
*/
int __myfs_mknod_implem(void *fsptr, size_t fssize, int *errnoptr, const char *path)
{
    // debug_ustar("mknod called");
    checkFsInit(fsptr, fssize, path);

    int status = init_fsRecord(fsptr, fssize, path, FALSE, 0);
    if(status == FAILURE)   //TODO: better communicate the exact error
    {
        *errnoptr = ENAMETOOLONG;
        return -1;
    }

    return 0;
}

/* Implements an emulation of the unlink system call for regular files
   on the filesystem of size fssize pointed to by fsptr.
   This function is called only for the deletion of regular files.
   On success, 0 is returned.
   On failure, -1 is returned and *errnoptr is set appropriately.
   The error codes are documented in man 2 unlink.
*/
int __myfs_unlink_implem(void *fsptr, size_t fssize, int *errnoptr,
                        const char *path) {

    // debug_ustar("unlink called");
    checkFsInit(fsptr, fssize, path);

    int index = findFileBlock(fsptr, fssize, path);
    if(index == FAILURE)
    {
        *errnoptr = EBADF;
        return -1;
    }

    struct fsRecord_header *h = (struct fsRecord_header *) fsptr;
    struct fsRecord_header *file = h+index;
    const size_t filesize = atol(file->fsSize);
    size_t numBlocks = calculateNumBlocksInFile(filesize);
    memset(file, 0, numBlocks * BLOCKSIZE);
    const char *dataSrc = ((char *) file) + numBlocks * BLOCKSIZE;
    const size_t copy_len = fssize - (dataSrc - (const char *) fsptr); //Hopefully
    memmove(file, dataSrc, copy_len);

    return 0;
}

/* Implements an emulation of the rmdir system call on the filesystem
   of size fssize pointed to by fsptr.
   The call deletes the directory indicated by path.
   On success, 0 is returned.
   On failure, -1 is returned and *errnoptr is set appropriately.
   The function call must fail when the directory indicated by path is
   not empty (if there are files or subdirectories other than . and ..).
   The error codes are documented in man 2 rmdir.
*/
int __myfs_rmdir_implem(void *fsptr, size_t fssize, int *errnoptr,
                        const char *path) {
    // debug_ustar("rmdir is called");
    checkFsInit(fsptr, fssize, path);

  return -1;
}

/* Implements an emulation of the mkdir system call on the filesystem
   of size fssize pointed to by fsptr.
   The call creates the directory indicated by path.
   On success, 0 is returned.
   On failure, -1 is returned and *errnoptr is set appropriately.
   The error codes are documented in man 2 mkdir.
*/
int __myfs_mkdir_implem(void *fsptr, size_t fssize, int *errnoptr, const char *path)
{
    // debug_ustar("mkdir called");
    checkFsInit(fsptr, fssize, path);

    int status = init_fsRecord(fsptr, fssize, path, TRUE, 0);
    if(status == FAILURE)
    {
        *errnoptr = ENAMETOOLONG;   //TODO: communicate the actual error
        return -1;
    }

    return 0;
}

/* Implements an emulation of the rename system call on the filesystem
   of size fssize pointed to by fsptr.
   The call moves the file or directory indicated by from to to.
   On success, 0 is returned.
   On failure, -1 is returned and *errnoptr is set appropriately.
   Caution: the function does more than what is hinted to by its name.
   In cases the from and to paths differ, the file is moved out of
   the from path and added to the to path.
   The error codes are documented in man 2 rename.
*/
int __myfs_rename_implem(void *fsptr, size_t fssize, int *errnoptr,
                         const char *from, const char *to) {

    // debug_ustar("fs rename called");
    checkFsInit(fsptr, fssize, from);

  return -1;
}

/* Implements an emulation of the truncate system call on the filesystem
   of size fssize pointed to by fsptr.
   The call changes the size of the file indicated by path to offset
   bytes.
   When the file becomes smaller due to the call, the extending bytes are
   removed. When it becomes larger, zeros are appended.
   On success, 0 is returned.
   On failure, -1 is returned and *errnoptr is set appropriately.
   The error codes are documented in man 2 truncate.
*/
int __myfs_truncate_implem(void *fsptr, size_t fssize, int *errnoptr,
                           const char *path, off_t offset) {

    // debug_ustar("trunc called");
    checkFsInit(fsptr, fssize, path);

    int index = findFileBlock(fsptr, fssize, path);
    if(index == FAILURE)
    {
        *errnoptr = EBADF;
        return -1;
    }

    struct fsRecord_header *h = (struct fsRecord_header *) fsptr;
    struct fsRecord_header *file = h+index;
    const size_t filesize = atol(file->fsSize);
    void *buffer = malloc(filesize);
    memcpy(buffer, file->padding, filesize);
    printf("buffer %s\n", (char*) buffer);
    if(__myfs_unlink_implem(fsptr, fssize, errnoptr, path) == FAILURE) return -1;  //unlink sets errno on its own
    if(init_fsRecord(fsptr, fssize, path, FALSE, offset) == FAILURE) return -1; //TODO: errno

    index = findFileBlock(fsptr, fssize, path);
    file = h+index;
    size_t bytesToCopy = (filesize < offset) ? filesize : offset;
    memcpy(file->padding, buffer, bytesToCopy);
    printf("dest %s\n", (char*) file->padding);
    free(buffer);

    return 0;
}

/* Implements an emulation of the open system call on the filesystem
   of size fssize pointed to by fsptr, without actually performing the opening
   of the file (no file descriptor is returned).
   The call just checks if the file (or directory) indicated by path
   can be accessed, i.e. if the path can be followed to an existing
   object for which the access rights are granted.
   On success, 0 is returned.
   On failure, -1 is returned and *errnoptr is set appropriately.
   The two only interesting error codes are
   * EFAULT: the filesystem is in a bad state, we can't do anything
   * ENOENT: the file that we are supposed to open doesn't exist (or a
             subpath).
   It is possible to restrict ourselves to only these two error
   conditions. It is also possible to implement more detailed error
   condition answers.
   The error codes are documented in man 2 open.
*/
int __myfs_open_implem(void *fsptr, size_t fssize, int *errnoptr,
                       const char *path) {

    // debug_ustar("fs open called");
    checkFsInit(fsptr, fssize, path);

    if(findFileBlock(fsptr, fssize, path) == FAILURE)
    {
        *errnoptr = ENOENT;
        return -1;
    }
    return 0;
}

/* Implements an emulation of the read system call on the filesystem
   of size fssize pointed to by fsptr.
   The call copies up to size bytes from the file indicated by
   path into the buffer, starting to read at offset. See the man page
   for read for the details when offset is beyond the end of the file etc.
   On success, the appropriate number of bytes read into the buffer is
   returned. The value zero is returned on an end-of-file condition.
   On failure, -1 is returned and *errnoptr is set appropriately.
   The error codes are documented in man 2 read.
*/
int __myfs_read_implem(void *fsptr, size_t fssize, int *errnoptr,
                       const char *path, char *buf, size_t size, off_t offset) {

    // debug_ustar("fs read called");
    checkFsInit(fsptr, fssize, path);

    int index = findFileBlock(fsptr, fssize, path);
    if(index == FAILURE)
    {
        *errnoptr = ENOENT;
        return -1;
    }

    const struct fsRecord_header *h = (const struct fsRecord_header *) fsptr;
    const struct fsRecord_header *file = h+index;
    const long filesize = atol(file->fsSize);
    if(offset > filesize) return 0; //EOF, TODO: possible off by one?

    const size_t bytesToRead = (filesize < size+offset) ? filesize : size+offset; //min(filesize, size+offset);
    memcpy(buf, (void *) file->padding, bytesToRead);
    printf("read %lu %s\n", bytesToRead, buf);
    return bytesToRead;
}

/* Implements an emulation of the write system call on the filesystem
   of size fssize pointed to by fsptr.
   The call copies up to size bytes to the file indicated by
   path into the buffer, starting to write at offset. See the man page
   for write for the details when offset is beyond the end of the file etc.
   On success, the appropriate number of bytes written into the file is
   returned. The value zero is returned on an end-of-file condition.
   On failure, -1 is returned and *errnoptr is set appropriately.
   The error codes are documented in man 2 write.
*/
int __myfs_write_implem(void *fsptr, size_t fssize, int *errnoptr,
                        const char *path, const char *buf, size_t size, off_t offset) {

    // debug_ustar("fs write called");
    checkFsInit(fsptr, fssize, path);

    int index = findFileBlock(fsptr, fssize, path);
    if(index == FAILURE)
    {
        *errnoptr = EBADF;
        return -1;
    }

    const struct fsRecord_header *h = (const struct fsRecord_header *) fsptr;
    const struct fsRecord_header *file = h+index;
    const long filesize = atol(file->fsSize);

    if(size + offset > filesize)
    {
        int status = __myfs_truncate_implem(fsptr, fssize, errnoptr, path, size + offset);
        if(status == FAILURE) return -1;    //trunc sets errno on its own
        index = findFileBlock(fsptr, fssize, path);
        file = h+index;
    }
    char *fileData = (char *) &(file->padding);
    char *writePtr = fileData + offset;
    memcpy(writePtr, buf, size);
    return size;
}

/* Implements an emulation of the utimensat system call on the filesystem
   of size fssize pointed to by fsptr.
   The call changes the access and modification times of the file
   or directory indicated by path to the values in ts.
   On success, 0 is returned.
   On failure, -1 is returned and *errnoptr is set appropriately.
   The error codes are documented in man 2 utimensat.
*/
int __myfs_utimens_implem(void *fsptr, size_t fssize, int *errnoptr,
                          const char *path, const struct timespec ts[2]) {

    // debug_ustar("utime stat called");
    checkFsInit(fsptr, fssize, path);
  /* STUB */
  return -1;
}

/* Implements an emulation of the statfs system call on the filesystem
   of size fssize pointed to by fsptr.
   The call gets information of the filesystem usage and puts in
   into stbuf.
   On success, 0 is returned.
   On failure, -1 is returned and *errnoptr is set appropriately.
   The error codes are documented in man 2 statfs.
   Essentially, only the following fields of struct statvfs need to be
   supported:
   f_bsize   fill with what you call a block (typically 1024 bytes)
   f_blocks  fill with the total number of blocks in the filesystem
   f_bfree   fill with the free number of blocks in the filesystem
   f_bavail  fill with same value as f_bfree
   f_namemax fill with your maximum file/directory name, if your
             filesystem has such a maximum
*/
int __myfs_statfs_implem(void *fsptr, size_t fssize, int *errnoptr,
                         struct statvfs* stbuf) {
    //TODO: Basic error checking. Not sure how/what to set errnoptr to.


    // debug_ustar("statfs called");

    const struct fsRecord_header *h = (const struct fsRecord_header *) fsptr;

    if(memcmp(h, "FIRES", 6) != 0) //file system has not been initialized
    {
        debug_ustar("checkFSInit: INITIALIZING FILESYSTEM");
        strncpy((char *)h, "FIRES", 6);
    }

    int usedBlocks = findNextFreeBlock(fsptr, fssize);
    size_t freeBlocks = fssize - ((usedBlocks - 1) * sizeof(struct fsRecord_header));

    stbuf->f_bsize = sizeof(struct fsRecord_header);
    stbuf->f_blocks = (fssize / sizeof(struct fsRecord_header));
    stbuf->f_bfree = (freeBlocks / sizeof(struct fsRecord_header));
    stbuf->f_bavail = (freeBlocks / sizeof(struct fsRecord_header));
    stbuf->f_namemax = sizeof(h->fName);

  //return -1;
  return 0;
}
