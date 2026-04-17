#include <mm/heap.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <stdint.h>
#include <utils/log.h>
#include <lib/string.h>

extern uint64_t hhdm_offset;

#define MAGIC_SLAB 0x800051AB
#define MAGIC_PAGE 0x80007A63

struct alloc_header {
    uint32_t magic;
    size_t pages;
    size_t size;
};

static uintptr_t kernel_heap_vaddr = 0xffffffff90000000;
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
    uintptr_t phys = pmm_alloc();
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

    size_t total_needed = size + sizeof(struct alloc_header);

    if (size > SLAB_1024) {
        size_t pages_count = (total_needed + 4095) / 4096;

        void* virt_start = (void*)kernel_heap_vaddr;
        kernel_heap_vaddr += (pages_count * 4096);

        pml4_t* pml4 = vmm_get_current();

        for (size_t i = 0; i < pages_count; i++) {
            uintptr_t phys = pmm_alloc();
            if (!phys) return NULL;
            vmm_map(pml4, (uintptr_t)virt_start + (i * 4096), phys, VMM_PRESENT | VMM_WRITE);
        }

        struct alloc_header* header = (struct alloc_header*)virt_start;
        header->magic = MAGIC_PAGE;
        header->pages = pages_count;
        header->size = size;

        return (void*)((uintptr_t)virt_start + sizeof(struct alloc_header));
    }

    struct slab_cache* cache = NULL;
    for (int i = 0; i < 6; i++) {
        if (total_needed <= caches[i].obj_size) {
            cache = &caches[i];
            break;
        }
    }

    if (!cache) return NULL;

    struct slab_header* slab = cache->first_slab;
    struct alloc_header* header = NULL;

    while (slab) {
        if (slab->free_list) {
            header = (struct alloc_header*)slab->free_list;
            slab->free_list = slab->free_list->next;
            break;
        }
        if (!slab->next_slab) slab->next_slab = create_slab(cache->obj_size);
        slab = slab->next_slab;
    }

    if (header) {
        header->magic = MAGIC_SLAB;
        header->size = size;
        return (void*)((uintptr_t)header + sizeof(struct alloc_header));
    }

    return NULL;
}

void kfree(void* ptr) {
    if (!ptr) return;

    struct alloc_header* header = (struct alloc_header*)((uintptr_t)ptr - sizeof(struct alloc_header));

    if (header->magic == MAGIC_SLAB) {
        struct slab_header* slab = (struct slab_header*)((uintptr_t)header & ~0xFFFULL);

        struct slab_object* obj = (struct slab_object*)header;
        obj->next = slab->free_list;
        slab->free_list = obj;
    } else if (header->magic == MAGIC_PAGE) {
        size_t pages_to_free = header->pages;
        uintptr_t virt_base = (uintptr_t)header;
        pml4_t* pml4 = vmm_get_current();

        for (size_t i = 0; i < pages_to_free; i++) {
            uintptr_t v_addr = virt_base + (i * 4096);

            uintptr_t phys = vmm_get_phys(pml4, v_addr);

            vmm_unmap(pml4, v_addr);
            if (phys) pmm_free(phys);
        }
    } else {
        log_error("KFREE", "Corruption détectée ! Magic invalide à %p", ptr);
    }
}

void* kcalloc(size_t count, size_t size) {
    void* ptr = kmalloc(count * size);
    if (ptr) memset(ptr, 0, count * size);
    return ptr;
}
