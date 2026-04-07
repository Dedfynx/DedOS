#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define VMM_PRESENT (1ULL << 0)
#define VMM_WRITE (1ULL << 1)
#define VMM_USER (1ULL << 2)
#define VMM_NX (1ULL << 63)

#define VMM_KERNEL (VMM_PRESENT | VMM_WRITE)
#define VMM_USERMODE (VMM_PRESENT | VMM_WRITE | VMM_USER)

typedef uint64_t pte_t;  // Page Table Entry
typedef pte_t pde_t;     // Page Directory Entry
typedef pte_t pdpte_t;   // PDPT Entry
typedef pte_t pml4e_t;   // PML4 Entry

typedef struct {
    pml4e_t entries[512];
} __attribute__((aligned(4096))) pml4_t;

void vmm_init(void);
void vmm_map(pml4_t* pml4, uint64_t virt, uint64_t phys, uint64_t flags);
void vmm_unmap(pml4_t* pml4, uint64_t virt);
pml4_t* vmm_new_pagemap(void);
void vmm_switch(pml4_t* pml4);
pml4_t* vmm_get_current(void);
uintptr_t vmm_get_phys(pml4_t* pml4, uint64_t virt);
