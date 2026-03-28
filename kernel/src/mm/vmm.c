// vmm.c
#include "kernel/log.h"
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <stdint.h>
#include <stddef.h>

extern uint64_t hhdm_offset;

static inline void* phys_to_virt(uint64_t phys) {
    return (void*)(phys + hhdm_offset);
}

static inline uint64_t virt_to_phys(void* virt) {
    return (uint64_t)virt - hhdm_offset;
}

static pte_t* get_or_create(pte_t* table, uint64_t index, uint64_t flags) {
    if (table[index] & VMM_PRESENT)
        return phys_to_virt(table[index] & ~0xFFFULL);

    void* phys = pmm_alloc();
    if (!phys) {
        log_error("VMM", "pmm_alloc failed");
        return NULL;
    }
    pte_t* virt = phys_to_virt((uint64_t)phys);
    for (int i = 0; i < 512; i++)
        virt[i] = 0;

    table[index] = (uint64_t)phys | flags;
    return virt;
}

void vmm_map(pml4_t* pml4, uint64_t virt, uint64_t phys, uint64_t flags) {
    uint64_t pml4_idx = (virt >> 39) & 0x1FF;
    uint64_t pdpt_idx = (virt >> 30) & 0x1FF;
    uint64_t pd_idx = (virt >> 21) & 0x1FF;
    uint64_t pt_idx = (virt >> 12) & 0x1FF;

    pte_t* pdpt = get_or_create(pml4->entries, pml4_idx, VMM_PRESENT | VMM_WRITE);
    if (!pdpt) return;
    pte_t* pd = get_or_create(pdpt, pdpt_idx, VMM_PRESENT | VMM_WRITE);
    if (!pd) return;
    pte_t* pt = get_or_create(pd, pd_idx, VMM_PRESENT | VMM_WRITE);
    if (!pt) return;

    pt[pt_idx] = phys | flags;
}

void vmm_unmap(pml4_t* pml4, uint64_t virt) {
    uint64_t pml4_idx = (virt >> 39) & 0x1FF;
    uint64_t pdpt_idx = (virt >> 30) & 0x1FF;
    uint64_t pd_idx = (virt >> 21) & 0x1FF;
    uint64_t pt_idx = (virt >> 12) & 0x1FF;

    if (!(pml4->entries[pml4_idx] & VMM_PRESENT)) return;
    pte_t* pdpt = phys_to_virt(pml4->entries[pml4_idx] & ~0xFFFULL);

    if (!(pdpt[pdpt_idx] & VMM_PRESENT)) return;
    pte_t* pd = phys_to_virt(pdpt[pdpt_idx] & ~0xFFFULL);

    if (!(pd[pd_idx] & VMM_PRESENT)) return;
    pte_t* pt = phys_to_virt(pd[pd_idx] & ~0xFFFULL);

    pt[pt_idx] = 0;
    __asm__ volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

pml4_t* vmm_new_pagemap(void) {
    pml4_t* pml4 = phys_to_virt((uint64_t)pmm_alloc());
    for (int i = 0; i < 512; i++)
        pml4->entries[i] = 0;
    return pml4;
}

void vmm_switch(pml4_t* pml4) {
    uint64_t phys = virt_to_phys(pml4);
    __asm__ volatile("mov %0, %%cr3" : : "r"(phys) : "memory");
}

pml4_t* vmm_get_current(void) {
    uint64_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    return phys_to_virt(cr3 & ~0xFFFULL);
}

void vmm_init(void) {
    pml4_t* pml4 = vmm_get_current();
    vmm_switch(pml4);
    log_info("VMM", "pagemap active");
}
