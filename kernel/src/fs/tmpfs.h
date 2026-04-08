#pragma once

#include <fs/vfs.h>
#include <stddef.h>

typedef struct tmpfs_node tmpfs_node_t;

void tmpfs_init(void);

tmpfs_node_t* tmpfs_create_file(tmpfs_node_t* parent, const char* name, void* data, size_t size);
tmpfs_node_t* tmpfs_create_dir(tmpfs_node_t* parent, const char* name);

tmpfs_node_t* tmpfs_get_root_internal(void);

struct tmpfs_node* tmpfs_add_entry(const char* full_path, void* data, size_t size, uint32_t flags);
