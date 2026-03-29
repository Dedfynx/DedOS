#include <mm/heap.h>
#include <mm/pmm.h>
#include <utils/log.h>
#include <libc/string.h>

extern uint64_t hhdm_offset;

struct slab_object {
    struct slab_object* next;
};

struct slab_header {
    size_t obj_size;
    struct slab_object* free_list;
    struct slab_header* next_slab;
};

struct slab_cache {
    size_t obj_size;
    struct slab_header* first_slab;
};

static struct slab_cache caches[6];

static struct slab_header* create_slab(size_t obj_size) {
    void* phys = pmm_alloc();
    if (!phys) return NULL;

    struct slab_header* slab = (struct slab_header*)((uint64_t)phys + hhdm_offset);

    slab->obj_size = obj_size;
    slab->next_slab = NULL;

    size_t available_space = 4096 - sizeof(struct slab_header);
    size_t num_objs = available_space / obj_size;

    uint8_t* first_obj_addr = (uint8_t*)slab + sizeof(struct slab_header);
    slab->free_list = (struct slab_object*)first_obj_addr;

    struct slab_object* current = slab->free_list;
    for (size_t i = 0; i < num_objs - 1; i++) {
        current->next = (struct slab_object*)((uint8_t*)current + obj_size);
        current = current->next;
    }
    current->next = NULL;
    return slab;
}

void heap_init(void) {
    size_t sizes[] = {SLAB_32, SLAB_64, SLAB_128, SLAB_256, SLAB_512, SLAB_1024};

    for (int i = 0; i < 6; i++) {
        caches[i].obj_size = sizes[i];
        caches[i].first_slab = create_slab(sizes[i]);
        if (!caches[i].first_slab) {
            log_error("HEAP", "Echec init cache %d bytes", sizes[i]);
        }
    }
    log_info("HEAP", "Slab allocator pret (HHDM)");
}

void* kmalloc(size_t size) {
    if (size == 0) return NULL;

    struct slab_cache* cache = NULL;
    for (int i = 0; i < 6; i++) {
        if (size <= caches[i].obj_size) {
            cache = &caches[i];
            break;
        }
    }

    if (!cache) {
        log_error("HEAP", "Allocation trop large (%d bytes) non supportee", size);
        return NULL;
    }

    struct slab_header* slab = cache->first_slab;
    while (slab) {
        if (slab->free_list) {
            struct slab_object* obj = slab->free_list;
            slab->free_list = obj->next;
            return (void*)obj;
        }

        if (!slab->next_slab) {
            slab->next_slab = create_slab(cache->obj_size);
        }
        slab = slab->next_slab;
    }

    return NULL;
}

void kfree(void* ptr) {
    if (!ptr) return;

    struct slab_header* slab = (struct slab_header*)((uint64_t)ptr & ~0xFFFULL);

    struct slab_object* obj = (struct slab_object*)ptr;
    obj->next = slab->free_list;
    slab->free_list = obj;
}

void* kcalloc(size_t count, size_t size) {
    void* ptr = kmalloc(count * size);
    if (ptr) memset(ptr, 0, count * size);
    return ptr;
}
