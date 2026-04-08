#include <stdint.h>
#include <stddef.h>

#define VFS_FILE (1 << 0)
#define VFS_DIRECTORY (1 << 1)
#define VFS_CHARDEVICE (1 << 2)

struct vfs_node;

typedef uint64_t (*vfs_read_t)(struct vfs_node* node, uint64_t offset, uint64_t size, void* buffer);
typedef uint64_t (*vfs_write_t)(struct vfs_node* node, uint64_t offset, uint64_t size, void* buffer);
typedef struct vfs_node* (*vfs_finddir_t)(struct vfs_node* node, const char* name);

typedef struct vfs_node {
    char name[128];
    uint32_t flags;
    uint64_t size;
    uint32_t inode;

    void* device;

    vfs_read_t read;
    vfs_write_t write;
    vfs_finddir_t finddir;
    struct vfs_node* ptr;
} vfs_node_t;

vfs_node_t* vfs_find_path(const char* path);
