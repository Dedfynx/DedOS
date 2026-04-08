#include <fs/tmpfs.h>
#include <fs/vfs.h>
#include <mm/heap.h>
#include <libc/string.h>
#include <utils/log.h>

struct tmpfs_node {
    char name[128];
    uint32_t flags;
    size_t size;
    uint8_t* data;

    struct tmpfs_node* next;
    struct tmpfs_node* children;
};

static uint64_t tmpfs_read(vfs_node_t* node, uint64_t offset, uint64_t size, void* buffer);
static vfs_node_t* tmpfs_finddir(vfs_node_t* dir, const char* name);

static vfs_node_t* tmpfs_create_vnode(struct tmpfs_node* internal) {
    if (!internal) return NULL;

    vfs_node_t* node = kmalloc(sizeof(vfs_node_t));
    memset(node, 0, sizeof(vfs_node_t));

    strcpy(node->name, internal->name);
    node->flags = internal->flags;
    node->size = internal->size;
    node->device = internal;

    node->read = tmpfs_read;
    node->finddir = tmpfs_finddir;

    return node;
}

static uint64_t tmpfs_read(vfs_node_t* node, uint64_t offset, uint64_t size, void* buffer) {
    struct tmpfs_node* internal = (struct tmpfs_node*)node->device;

    if (offset >= internal->size) return 0;
    if (offset + size > internal->size) size = internal->size - offset;

    memcpy(buffer, internal->data + offset, size);
    return size;
}

static vfs_node_t* tmpfs_finddir(vfs_node_t* dir, const char* name) {
    struct tmpfs_node* internal_dir = (struct tmpfs_node*)dir->device;

    if (internal_dir->flags != VFS_DIRECTORY) return NULL;

    struct tmpfs_node* current = internal_dir->children;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return tmpfs_create_vnode(current);
        }
        current = current->next;
    }
    return NULL;
}

static struct tmpfs_node* root_internal = NULL;

void tmpfs_init() {
    root_internal = kmalloc(sizeof(struct tmpfs_node));
    memset(root_internal, 0, sizeof(struct tmpfs_node));

    strcpy(root_internal->name, "/");
    root_internal->flags = VFS_DIRECTORY;

    vfs_root = tmpfs_create_vnode(root_internal);
}

struct tmpfs_node* tmpfs_create_file(struct tmpfs_node* parent, const char* name, void* data, size_t size) {
    if (!parent || parent->flags != VFS_DIRECTORY) return NULL;

    struct tmpfs_node* new_node = kmalloc(sizeof(struct tmpfs_node));
    memset(new_node, 0, sizeof(struct tmpfs_node));

    strcpy(new_node->name, name);
    new_node->flags = VFS_FILE;
    new_node->size = size;

    if (size > 0) {
        new_node->data = kmalloc(size);
        memcpy(new_node->data, data, size);
    }

    new_node->next = parent->children;
    parent->children = new_node;

    return new_node;
}

struct tmpfs_node* tmpfs_create_dir(struct tmpfs_node* parent, const char* name) {
    struct tmpfs_node* new_dir = tmpfs_create_file(parent, name, NULL, 0);
    if (new_dir) {
        new_dir->flags = VFS_DIRECTORY;
    }
    return new_dir;
}

struct tmpfs_node* tmpfs_get_root_internal() {
    return root_internal;
}
