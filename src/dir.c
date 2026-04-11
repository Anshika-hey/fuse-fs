#include "fs.h"

int dir_add_child(int parent_idx, int child_idx)
{
    Inode *parent = &inode_table[parent_idx];
    if (parent->nchildren >= MAX_CHILDREN)
        return -ENOSPC;
    parent->children[parent->nchildren++] = child_idx;
    inode_table[child_idx].parent = parent_idx;
    inode_update_times(parent_idx, 0, 1, 1);
    return 0;
}

int dir_remove_child(int parent_idx, int child_idx)
{
    Inode *parent = &inode_table[parent_idx];
    for (int i = 0; i < parent->nchildren; i++)
    {
        if (parent->children[i] == child_idx)
        {
            for (int j = i; j < parent->nchildren - 1; j++)
                parent->children[j] = parent->children[j + 1];
            parent->nchildren--;
            inode_update_times(parent_idx, 0, 1, 1);
            return 0;
        }
    }
    return -ENOENT;
}

int dir_find_child(int parent_idx, const char *name)
{
    Inode *parent = &inode_table[parent_idx];
    for (int i = 0; i < parent->nchildren; i++)
    {
        int cidx = parent->children[i];
        if (inode_table[cidx].type != INODE_FREE &&
            strcmp(inode_table[cidx].name, name) == 0)
        {
            return cidx;
        }
    }
    return -1;
}

void dir_get_parent_path(const char *path, char *parent_path)
{
    strncpy(parent_path, path, MAX_PATH - 1);
    char *last_slash = strrchr(parent_path, '/');
    if (last_slash == parent_path)
    {
        parent_path[1] = '\0';
    }
    else if (last_slash)
    {
        *last_slash = '\0';
    }
}

void dir_get_basename(const char *path, char *basename_out)
{
    const char *last_slash = strrchr(path, '/');
    if (last_slash)
        strncpy(basename_out, last_slash + 1, MAX_FILENAME - 1);
    else
        strncpy(basename_out, path, MAX_FILENAME - 1);
}