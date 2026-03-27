#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>
#include <flanterm.h>
#include <flanterm_backends/fb.h>

#include "kprintf.h"

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

// Halt and catch fire function.
static void hcf(void) {
    for (;;) {
        asm("hlt");
    }
}

struct flanterm_context* ft_ctx;

// The following will be our kernel's entry point.
// If renaming kmain() to something else, make sure to change the
// linker script accordingly.
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

    size_t H = framebuffer->height;
    size_t W = framebuffer->width;

    // Note: we assume the framebuffer model is RGB with 32-bit pixels.
    for (size_t i = 0; i < H; i++) {
        for (size_t j = 0; j < W; j++) {
            volatile uint32_t* fb_ptr = framebuffer->address;
            uint8_t red, green, blue;
            red = (j * 2) ^ (i * 2);
            blue = i ^ j;
            green = (j * 4) ^ (i * 4);
            fb_ptr[i * (framebuffer->pitch / 4) + j] = (((red << 16) & 0xFF0000) | ((green << 8) & 0x00FF00) | (blue & 0x0000FF));
        }
    }

    kprintf("DedOS v0.1\n");
    kprintf("Framebuffer: %u x %u\n", framebuffer->width, framebuffer->height);
    kprintf("Adresse kernel: %p\n", &kmain);
    // Test
    kprintf("Valeur : %d (hex: %x)\n", 255, 255);
    kprintf("Pointeur : %p\n", (void*)0x12345678);
    kprintf("Pourcentage : 50%%\n");

    // We're done, just hang...
    hcf();
}
