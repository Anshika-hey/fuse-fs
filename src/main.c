#define FUSE_USE_VERSION 26
#include "fs.h"

static const struct fuse_operations fs_ops = {
    .getattr = fs_getattr,
    .readdir = fs_readdir,
    .open = fs_open,
    .read = fs_read,
    .write = fs_write,
    .create = fs_create,
    .mkdir = fs_mkdir,
    .unlink = fs_unlink,
    .rmdir = fs_rmdir,
    .truncate = fs_truncate,
    .rename = fs_rename,
    .chmod = fs_chmod,
    .utimens = fs_utimens,
};

static void fs_init_root(void)
{
    memset(inode_table, 0, sizeof(inode_table));
    int root = inode_alloc();
    inode_table[root].type = INODE_DIR;
    inode_table[root].mode = S_IFDIR | 0755;
    inode_table[root].parent = -1;
    strncpy(inode_table[root].path, "/", MAX_PATH - 1);
    strncpy(inode_table[root].name, "/", MAX_FILENAME - 1);
    int hello = inode_create("/hello.txt", INODE_FILE, S_IFREG | 0644);
    const char *msg = "Hello from FuseFS!\n";
    memcpy(inode_table[hello].data, msg, strlen(msg));
    inode_table[hello].size = strlen(msg);
    dir_add_child(0, hello);
}

int main(int argc, char *argv[])
{
    fs_init_root();
    return fuse_main(argc, argv, &fs_ops, NULL);
}