#include "fs.h"

int fs_getattr(const char *path, struct stat *stbuf)
{
    memset(stbuf, 0, sizeof(struct stat));
    int idx = inode_find_by_path(path);
    if (idx < 0)
        return -ENOENT;
    Inode *n = &inode_table[idx];
    stbuf->st_mode = n->mode;
    stbuf->st_nlink = (n->type == INODE_DIR) ? 2 : 1;
    stbuf->st_uid = n->uid;
    stbuf->st_gid = n->gid;
    stbuf->st_size = n->size;
    stbuf->st_atime = n->atime;
    stbuf->st_mtime = n->mtime;
    stbuf->st_ctime = n->ctime;
    return 0;
}

int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
               off_t offset, struct fuse_file_info *fi)
{
    (void)offset;
    (void)fi;
    int idx = inode_find_by_path(path);
    if (idx < 0)
        return -ENOENT;
    if (inode_table[idx].type != INODE_DIR)
        return -ENOTDIR;
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    Inode *dir = &inode_table[idx];
    for (int i = 0; i < dir->nchildren; i++)
    {
        int c = dir->children[i];
        if (inode_table[c].type != INODE_FREE)
            filler(buf, inode_table[c].name, NULL, 0);
    }
    return 0;
}

int fs_open(const char *path, struct fuse_file_info *fi)
{
    (void)fi;
    int idx = inode_find_by_path(path);
    if (idx < 0)
        return -ENOENT;
    return 0;
}

int fs_read(const char *path, char *buf, size_t size, off_t offset,
            struct fuse_file_info *fi)
{
    (void)fi;
    int idx = inode_find_by_path(path);
    if (idx < 0)
        return -ENOENT;
    Inode *n = &inode_table[idx];
    if (offset >= n->size)
        return 0;
    size_t to_read = size;
    if ((size_t)(offset + size) > (size_t)n->size)
        to_read = n->size - offset;
    memcpy(buf, n->data + offset, to_read);
    return (int)to_read;
}

int fs_write(const char *path, const char *buf, size_t size, off_t offset,
             struct fuse_file_info *fi)
{
    (void)fi;
    int idx = inode_find_by_path(path);
    if (idx < 0)
        return -ENOENT;
    Inode *n = &inode_table[idx];
    if (offset + (off_t)size > MAX_FILE_SIZE)
        return -EFBIG;
    memcpy(n->data + offset, buf, size);
    if (offset + (off_t)size > n->size)
        n->size = offset + size;
    return (int)size;
}

int fs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    (void)fi;
    if (inode_find_by_path(path) >= 0)
        return -EEXIST;
    char parent_path[MAX_PATH];
    dir_get_parent_path(path, parent_path);
    int pidx = inode_find_by_path(parent_path);
    if (pidx < 0)
        return -ENOENT;
    int idx = inode_create(path, INODE_FILE, S_IFREG | mode);
    if (idx < 0)
        return -ENOSPC;
    return dir_add_child(pidx, idx);
}

int fs_mkdir(const char *path, mode_t mode)
{
    if (inode_find_by_path(path) >= 0)
        return -EEXIST;
    char parent_path[MAX_PATH];
    dir_get_parent_path(path, parent_path);
    int pidx = inode_find_by_path(parent_path);
    if (pidx < 0)
        return -ENOENT;
    int idx = inode_create(path, INODE_DIR, S_IFDIR | mode);
    if (idx < 0)
        return -ENOSPC;
    return dir_add_child(pidx, idx);
}

int fs_unlink(const char *path)
{
    int idx = inode_find_by_path(path);
    if (idx < 0)
        return -ENOENT;
    int pidx = inode_table[idx].parent;
    if (pidx >= 0)
        dir_remove_child(pidx, idx);
    inode_free(idx);
    return 0;
}

int fs_rmdir(const char *path)
{
    int idx = inode_find_by_path(path);
    if (idx < 0)
        return -ENOENT;
    if (inode_table[idx].nchildren > 0)
        return -ENOTEMPTY;
    int pidx = inode_table[idx].parent;
    if (pidx >= 0)
        dir_remove_child(pidx, idx);
    inode_free(idx);
    return 0;
}

int fs_truncate(const char *path, off_t size)
{
    int idx = inode_find_by_path(path);
    if (idx < 0)
        return -ENOENT;
    inode_table[idx].size = size;
    return 0;
}

int fs_rename(const char *from, const char *to)
{
    int idx = inode_find_by_path(from);
    if (idx < 0)
        return -ENOENT;
    int old_parent = inode_table[idx].parent;
    if (old_parent >= 0)
        dir_remove_child(old_parent, idx);
    strncpy(inode_table[idx].path, to, MAX_PATH - 1);
    dir_get_basename(to, inode_table[idx].name);
    char new_parent_path[MAX_PATH];
    dir_get_parent_path(to, new_parent_path);
    int new_parent = inode_find_by_path(new_parent_path);
    if (new_parent >= 0)
        dir_add_child(new_parent, idx);
    return 0;
}

int fs_chmod(const char *path, mode_t mode)
{
    int idx = inode_find_by_path(path);
    if (idx < 0)
        return -ENOENT;
    inode_table[idx].mode = (inode_table[idx].mode & S_IFMT) | mode;
    return 0;
}

int fs_utimens(const char *path, const struct timespec tv[2])
{
    int idx = inode_find_by_path(path);
    if (idx < 0)
        return -ENOENT;
    inode_table[idx].atime = tv[0].tv_sec;
    inode_table[idx].mtime = tv[1].tv_sec;
    return 0;
}