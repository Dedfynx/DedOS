#include "fs/initrd.h"
#include "fs/tmpfs.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>
#include <flanterm.h>
#include <flanterm_backends/fb.h>

#include <utils/kprintf.h>
#include <utils/log.h>
#include <arch/x86_64/gdt.h>
#include <arch/x86_64/idt.h>
#include <arch/x86_64/pic.h>
#include <arch/x86_64/apic/lapic.h>
#include <arch/x86_64/apic/ioapic.h>
#include <arch/x86_64/apic/lapic_timer.h>
#include <drivers/keyboard.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/heap.h>
#include <acpi/acpi.h>
#include <acpi/madt.h>
#include <libc/string.h>
#include <scheduler/scheduler.h>
#include <scheduler/process.h>
#include <scheduler/thread.h>
#include <fs/tmpfs.h>
#include <fs/vfs.h>

// Set the base revision to 6, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.
__attribute__((used, section(".limine_requests"))) static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(6);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.
__attribute__((used, section(".limine_requests"))) static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0};

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.
__attribute__((used, section(".limine_requests_start"))) static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end"))) static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

__attribute__((used, section(".limine_requests"))) static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0};

__attribute__((used, section(".limine_requests"))) static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0};

__attribute__((used, section(".limine_requests"))) static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST_ID,
    .revision = 0};

// Halt and catch fire function.
static void hcf(void) {
    for (;;) {
        asm("hlt");
    }
}

//
void task_a(void) {
    int i = 0;
    while (i < 6) {
        log_info("TASK_A", "Iter : %d", i);
        i++;
        thread_sleep(500);
    }
}

void task_b(void) {
    int i = 0;
    while (i < 3) {
        log_info("TASK_B", "Iter : %d", i);
        i++;
        thread_sleep(1000);
    }
}

struct flanterm_context* ft_ctx;

void kmain(void) {
    // Ensure the bootloader actually understands our base revision (see spec).
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
        hcf();
    }

    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Fetch the first framebuffer.
    struct limine_framebuffer* framebuffer = framebuffer_request.response->framebuffers[0];

    ft_ctx = flanterm_fb_init(
        NULL,
        NULL,
        framebuffer->address,
        framebuffer->width,
        framebuffer->height,
        framebuffer->pitch,
        framebuffer->red_mask_size, framebuffer->red_mask_shift,
        framebuffer->green_mask_size, framebuffer->green_mask_shift,
        framebuffer->blue_mask_size, framebuffer->blue_mask_shift,
        NULL,
        NULL, NULL,
        NULL, NULL,
        NULL, NULL,
        NULL, 0, 0, 1,
        0, 0,
        0,
        0);

    // Background Test
    size_t H = framebuffer->height;
    size_t W = framebuffer->width;
    volatile uint32_t* fb_ptr = framebuffer->address;
    for (size_t i = 0; i < H; i++) {
        for (size_t j = 0; j < W; j++) {
            uint8_t red = (j * 2) ^ (i * 2);
            uint8_t blue = i ^ j;
            uint8_t green = (j * 4) ^ (i * 4);
            fb_ptr[i * (framebuffer->pitch / 4) + j] = (((red << 16) & 0xFF0000) | ((green << 8) & 0x00FF00) | (blue & 0x0000FF));
        }
    }
    gdt_init();
    idt_init();
    pmm_init(memmap_request.response, hhdm_request.response->offset);
    vmm_init();
    if (rsdp_request.response == NULL) {
        log_error("ACPI", "pas de RSDP !");
        hcf();
    }
    heap_init();
    acpi_init(rsdp_request.response->address);
    acpi_sdt_header_t* madt = acpi_find_table("APIC");
    if (madt)
        log_debug("ACPI", "MADT trouvé à %p", madt);
    else
        log_error("ACPI", "MADT non trouvé");
    madt_init();
    lapic_init();
    ioapic_init();
    lapic_timer_init();
    pic_disable();
    ioapic_redirect(1, 33, 0);
    ioapic_unmask(1);

    ioapic_redirect(0, 32, 0);
    ioapic_unmask(0);
    keyboard_init();
    log_info("KERNEL", "Clavier initialisé");

    __asm__ volatile("sti");

    log_info("KERNEL", "DedOS v0.9");
    log_debug("TEST", "Framebuffer: %u x %u", framebuffer->width, framebuffer->height);
    log_debug("TEST", "Adresse kernel: %p", &kmain);

    // Test kmalloc
    void* a = kmalloc(64);
    void* b = kmalloc(128);
    log_debug("TEST", "KMalloc a=%p b=%p", a, b);
    kfree(a);
    void* c = kmalloc(64);
    log_debug("TEST", "KMalloc after free c=%p", c);
    kfree(b);
    kfree(c);

    tmpfs_init();
    initrd_init();

    // test filesystem

    vfs_node_t* FILE = vfs_find_path("/etc/test.txt");
    if (FILE) {
        log_info("VFS", "Fichier /etc/test.txt trouvé");
        char test_buf[16];
        vfs_read(FILE, 0, 13, test_buf);
        log_debug("VFS", "Contenu : %s", test_buf);
        vfs_close(FILE);
    } else {
        log_error("VFS", "Echec du find_path");
    }

    scheduler_init();
    scheduler_lock();
    process_t* proc_a = process_create("TASK_A");
    thread_t* th_a = thread_create(proc_a, task_a);
    scheduler_add_thread(th_a);

    process_t* proc_b = process_create("TASK_B");
    thread_t* th_b = thread_create(proc_b, task_b);
    scheduler_add_thread(th_b);

    scheduler_unlock();
    // We're done, just hang...
    hcf();
}
