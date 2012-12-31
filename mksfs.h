/* 
 * File:   mksfs.h
 * Author: saqib khalil yawar
 *
 * Created on November 4, 2009, 8:48 PM
 */

#ifndef _MKSFS_H
#define	_MKSFS_H

#ifdef	__cplusplus
extern "C" {
#endif

#ifdef	__cplusplus
}
#endif

#endif	/* _MKSFS_H */
int searchBlockSpace();
int searchFAT();
int findDirectory();
int findFDT();
int updateDisk();
void mksfs(int fresh);
int sfs_open(char *name);
void sfs_write(int fd, char *buf, int length);
void sfs_read(int fd, char *buf, int length);
void sfs_ls();
int sfs_close(int fd);
int sfs_remove(char *file);
