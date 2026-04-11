#ifndef FS_H
#define FS_H

#define FUSE_USE_VERSION 26
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_FILES 128
#define MAX_FILENAME 256
#define MAX_PATH 1024
#define BLOCK_SIZE 4096
#define MAX_FILE_SIZE (BLOCK_SIZE * 16)
#define MAX_CHILDREN 64

typedef enum
{
    INODE_FREE = 0,
    INODE_FILE,
    INODE_DIR
} InodeType;

typedef struct Inode
{
    InodeType type;
    char name[MAX_FILENAME];
    char path[MAX_PATH];
    mode_t mode;
    uid_t uid;
    gid_t gid;
    off_t size;
    time_t atime, mtime, ctime;
    char *data;
    int children[MAX_CHILDREN];
    int nchildren;
    int parent;
} Inode;

extern Inode inode_table[MAX_FILES];
extern int inode_count;

int inode_alloc(void);
void inode_free(int idx);
int inode_find_by_path(const char *path);
int inode_create(const char *path, InodeType type, mode_t mode);
void inode_update_times(int idx, int access, int modify, int change);

int dir_add_child(int parent_idx, int child_idx);
int dir_remove_child(int parent_idx, int child_idx);
void dir_get_parent_path(const char *path, char *parent_path);
void dir_get_basename(const char *path, char *basename_out);

int fs_getattr(const char *path, struct stat *stbuf);
int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
               off_t offset, struct fuse_file_info *fi);
int fs_open(const char *path, struct fuse_file_info *fi);
int fs_read(const char *path, char *buf, size_t size, off_t offset,
            struct fuse_file_info *fi);
int fs_write(const char *path, const char *buf, size_t size, off_t offset,
             struct fuse_file_info *fi);
int fs_create(const char *path, mode_t mode, struct fuse_file_info *fi);
int fs_mkdir(const char *path, mode_t mode);
int fs_unlink(const char *path);
int fs_rmdir(const char *path);
int fs_truncate(const char *path, off_t size);
int fs_rename(const char *from, const char *to);
int fs_chmod(const char *path, mode_t mode);
int fs_utimens(const char *path, const struct timespec tv[2]);

#endif