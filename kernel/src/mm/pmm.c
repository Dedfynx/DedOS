#include <utils/log.h>
#include <mm/pmm.h>
#include <limine.h>
#include <stdint.h>
#include <stddef.h>

#define BITMAP_SET(bit) (bitmap[(bit) / 8] |= (1 << ((bit) % 8)))
#define BITMAP_CLEAR(bit) (bitmap[(bit) / 8] &= ~(1 << ((bit) % 8)))
#define BITMAP_TEST(bit) (bitmap[(bit) / 8] & (1 << ((bit) % 8)))

static uint8_t* bitmap = NULL;
static size_t total_pages = 0;
static size_t free_pages = 0;
static size_t bitmap_size = 0;
uint64_t hhdm_offset = 0;

void pmm_init(struct limine_memmap_response* memmap, uint64_t hhdm) {
    hhdm_offset = hhdm;

    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap->entries[i];
    }

    uint64_t highest = 0;
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            if (entry->base + entry->length > highest)
                highest = entry->base + entry->length;
        }
    }

    total_pages = highest / PAGE_SIZE;
    bitmap_size = (total_pages + 7) / 8;

    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE && entry->length >= bitmap_size) {
            bitmap = (uint8_t*)(entry->base + hhdm_offset);
            break;
        }
    }

    if (!bitmap) {
        log_error("PMM", "impossible de placer le bitmap !");
        return;
    }

    for (size_t i = 0; i < bitmap_size; i++)
        bitmap[i] = 0xFF;

    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            uint64_t base = entry->base / PAGE_SIZE;
            uint64_t pages = entry->length / PAGE_SIZE;
            for (uint64_t j = 0; j < pages; j++) {
                BITMAP_CLEAR(base + j);
                free_pages++;
            }
        }
    }

    uint64_t bitmap_pages = (bitmap_size + PAGE_SIZE - 1) / PAGE_SIZE;
    uint64_t bitmap_phys = (uint64_t)bitmap - hhdm_offset;
    uint64_t bitmap_base = bitmap_phys / PAGE_SIZE;
    for (uint64_t i = 0; i < bitmap_pages; i++) {
        BITMAP_SET(bitmap_base + i);
        free_pages--;
    }

    log_info("PMM", "%u pages libres / %u total (%u MB libres)",
        free_pages, total_pages,
        (free_pages * PAGE_SIZE) / (1024 * 1024));
}

uintptr_t pmm_alloc(void) {
    for (size_t i = 0; i < total_pages; i++) {
        if (!BITMAP_TEST(i)) {
            BITMAP_SET(i);
            free_pages--;
            return (uintptr_t)(uint64_t)(i * PAGE_SIZE);
        }
    }
    log_error("PMM", "plus de memoire physique !");
    return 0;
}

void pmm_free(uintptr_t ptr) {
    uint64_t page = (uint64_t)ptr / PAGE_SIZE;
    if (BITMAP_TEST(page)) {
        BITMAP_CLEAR(page);
        free_pages++;
    }
}

size_t pmm_free_pages(void) { return free_pages; }
size_t pmm_total_pages(void) { return total_pages; }
