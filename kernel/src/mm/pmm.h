#pragma once
#include <stdint.h>
#include <stddef.h>
#include <limine.h>

#define PAGE_SIZE 4096

extern uint64_t hhdm_offset;

void pmm_init(struct limine_memmap_response* memmap, uint64_t hhdm);
void* pmm_alloc(void);
void pmm_free(void* ptr);
size_t pmm_free_pages(void);
size_t pmm_total_pages(void);
