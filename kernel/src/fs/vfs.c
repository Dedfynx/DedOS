#include <fs/vfs.h>
#include <utils/log.h>

vfs_node_t* vfs_root = NULL;

uint64_t vfs_read(vfs_node_t* node, uint64_t offset, uint64_t size, void* buffer) {
    if (node->read)
        return node->read(node, offset, size, buffer);
    return 0;
}

vfs_node_t* vfs_find_path(const char* path) {
    if (!path || path[0] == '\0') return NULL;
    if (path[0] == '/' && path[1] == '\0') return vfs_root;

    vfs_node_t* current_node = vfs_root;
    uint8_t cursor = 0;
    if (path[0] == '/') {
        cursor = 1;
    }

    while (path[cursor] != '\0') {
        char name[128];
        int i = 0;

        while (path[cursor] != '/' && path[cursor] != '\0') {
            if (i < 127) {
                name[i++] = path[i];
            }
            cursor++;
        }
        name[i] = '\0';

        if (i == 0) {
            if (path[cursor] == '/') {
                cursor++;
            }
            continue;
        }

        if (!(current_node->flags & VFS_DIRECTORY)) {
            log_error("VFS", "Tentative de finddir sur un fichier: %s", current_node->name);
            return NULL;
        }

        vfs_node_t* next = current_node->finddir(current_node, name);
        if (!next) {
            log_debug("VFS", "Composant non trouve: %s", name);
            return NULL;
        }

        current_node = next;

        if (path[cursor] == '/') {
            cursor++;
        }
    }
    return current_node;
}
