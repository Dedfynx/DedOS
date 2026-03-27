#include <pic.h>
#include <io.h>

void pic_init(void) {
    // Sauvegarde les masques
    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);

    // ICW1 : init en cascade
    outb(PIC1_COMMAND, 0x11);
    io_wait();
    outb(PIC2_COMMAND, 0x11);
    io_wait();

    // ICW2 : remapping — IRQ0-7 → 32-39, IRQ8-15 → 40-47
    outb(PIC1_DATA, 0x20);
    io_wait();  // master offset = 32
    outb(PIC2_DATA, 0x28);
    io_wait();  // slave offset  = 40

    // ICW3 : cascade
    outb(PIC1_DATA, 0x04);
    io_wait();  // master : slave sur IRQ2
    outb(PIC2_DATA, 0x02);
    io_wait();  // slave  : cascade identity

    // ICW4 : mode 8086
    outb(PIC1_DATA, 0x01);
    io_wait();
    outb(PIC2_DATA, 0x01);
    io_wait();

    // Restaure les masques
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

void pic_eoi(uint8_t irq) {
    if (irq >= 8)
        outb(PIC2_COMMAND, PIC_EOI);
    outb(PIC1_COMMAND, PIC_EOI);
}

void pic_mask(uint8_t irq) {
    uint16_t port;
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    outb(port, inb(port) | (1 << irq));
}

void pic_unmask(uint8_t irq) {
    uint16_t port;
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    outb(port, inb(port) & ~(1 << irq));
}
