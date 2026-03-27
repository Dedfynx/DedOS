#include <pmm.h>
#include <limine.h>
#include <kprintf.h>
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
    kprintf("PMM: hhdm_offset = %p\n", (void*)hhdm_offset);

    // Affiche toutes les entrees memmap
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap->entries[i];
        kprintf("  [%u] base=%p len=%p type=%u\n",
            (uint32_t)i,
            (void*)entry->base,
            (void*)entry->length,
            (uint32_t)entry->type);
    }

    // 1. Trouve la plus haute adresse physique USABLE
    uint64_t highest = 0;
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            if (entry->base + entry->length > highest)
                highest = entry->base + entry->length;
        }
    }
    kprintf("PMM: highest = %p\n", (void*)highest);

    // 2. Calcule la taille du bitmap
    total_pages = highest / PAGE_SIZE;
    bitmap_size = (total_pages + 7) / 8;
    kprintf("PMM: total_pages = %u bitmap_size = %u\n", total_pages, bitmap_size);

    // 3. Trouve un endroit pour stocker le bitmap
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE && entry->length >= bitmap_size) {
            bitmap = (uint8_t*)(entry->base + hhdm_offset);
            kprintf("PMM: bitmap place a %p (phys: %p)\n",
                (void*)bitmap, (void*)entry->base);
            break;
        }
    }

    if (!bitmap) {
        kprintf("PMM: impossible de placer le bitmap !\n");
        return;
    }

    // 4. Test ecriture avant de continuer
    kprintf("PMM: test ecriture bitmap...\n");
    bitmap[0] = 0xFF;
    kprintf("PMM: ecriture ok\n");

    // 4. Marque tout comme occupe
    for (size_t i = 0; i < bitmap_size; i++)
        bitmap[i] = 0xFF;

    // 5. Marque les zones USABLE comme libres
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

    // 6. Marque les pages du bitmap comme occupees
    uint64_t bitmap_pages = (bitmap_size + PAGE_SIZE - 1) / PAGE_SIZE;
    uint64_t bitmap_phys = (uint64_t)bitmap - hhdm_offset;
    uint64_t bitmap_base = bitmap_phys / PAGE_SIZE;
    for (uint64_t i = 0; i < bitmap_pages; i++) {
        BITMAP_SET(bitmap_base + i);
        free_pages--;
    }

    kprintf("PMM: %u pages libres / %u total (%u MB libres)\n",
        free_pages, total_pages,
        (free_pages * PAGE_SIZE) / (1024 * 1024));
}

void* pmm_alloc(void) {
    for (size_t i = 0; i < total_pages; i++) {
        if (!BITMAP_TEST(i)) {
            BITMAP_SET(i);
            free_pages--;
            return (void*)(uint64_t)(i * PAGE_SIZE);
        }
    }
    kprintf("PMM: plus de memoire physique !\n");
    return NULL;
}

void pmm_free(void* ptr) {
    uint64_t page = (uint64_t)ptr / PAGE_SIZE;
    if (BITMAP_TEST(page)) {
        BITMAP_CLEAR(page);
        free_pages++;
    }
}

size_t pmm_free_pages(void) { return free_pages; }
size_t pmm_total_pages(void) { return total_pages; }
