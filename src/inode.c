#include "fs.h"

Inode inode_table[MAX_FILES];
int inode_count = 0;

int inode_alloc(void)
{
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (inode_table[i].type == INODE_FREE)
        {
            memset(&inode_table[i], 0, sizeof(Inode));
            inode_table[i].uid = getuid();
            inode_table[i].gid = getgid();
            inode_table[i].atime = time(NULL);
            inode_table[i].mtime = time(NULL);
            inode_table[i].ctime = time(NULL);
            inode_table[i].parent = -1;
            inode_count++;
            return i;
        }
    }
    return -1;
}

void inode_free(int idx)
{
    if (idx < 0 || idx >= MAX_FILES)
        return;
    if (inode_table[idx].data)
    {
        free(inode_table[idx].data);
        inode_table[idx].data = NULL;
    }
    inode_table[idx].type = INODE_FREE;
    inode_count--;
}

int inode_find_by_path(const char *path)
{
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (inode_table[i].type != INODE_FREE &&
            strcmp(inode_table[i].path, path) == 0)
        {
            return i;
        }
    }
    return -1;
}

int inode_create(const char *path, InodeType type, mode_t mode)
{
    int idx = inode_alloc();
    if (idx < 0)
        return -1;

    strncpy(inode_table[idx].path, path, MAX_PATH - 1);
    inode_table[idx].type = type;
    inode_table[idx].mode = mode;
    inode_table[idx].size = 0;

    dir_get_basename(path, inode_table[idx].name);

    if (type == INODE_FILE)
    {
        inode_table[idx].data = calloc(1, MAX_FILE_SIZE);
        if (!inode_table[idx].data)
        {
            inode_free(idx);
            return -1;
        }
    }
    return idx;
}

void inode_update_times(int idx, int access, int modify, int change)
{
    time_t now = time(NULL);
    if (access)
        inode_table[idx].atime = now;
    if (modify)
        inode_table[idx].mtime = now;
    if (change)
        inode_table[idx].ctime = now;
}