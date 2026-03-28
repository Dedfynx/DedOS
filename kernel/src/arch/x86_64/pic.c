#include <arch/x86_64/pic.h>
#include <arch/x86_64/io.h>
#include <stdint.h>

void pic_init(void) {
    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);
    uint8_t offset1 = 0x20;
    uint8_t offset2 = 0x28;

    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
    io_wait();                                  //
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);  //
    io_wait();                                  //
    outb(PIC1_DATA, offset1);                   // ICW2: Master PIC vector offset
    io_wait();                                  //
    outb(PIC2_DATA, offset2);                   // ICW2: Slave PIC vector offset
    io_wait();                                  //
    outb(PIC1_DATA, 0x04);                      // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    io_wait();                                  //
    outb(PIC2_DATA, 0x02);                      // ICW3: tell Slave PIC its cascade identity (0000 0010)
    io_wait();                                  //
    outb(PIC1_DATA, ICW4_8086);                 // ICW4: have the PICs use 8086 mode (and not 8080 mode)
    io_wait();                                  //
    outb(PIC2_DATA, ICW4_8086);                 //
    io_wait();                                  //

    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

void pic_sendEOI(uint8_t irq) {
    if (irq >= 8)
        outb(PIC2_COMMAND, PIC_EOI);
    outb(PIC1_COMMAND, PIC_EOI);
}

void pic_mask(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    value = inb(port) | (1 << irq);
    outb(port, value);
}

void pic_unmask(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    value = inb(port) & ~(1 << irq);
    outb(port, value);
}

void pic_disable(void) {
    // Masque toutes les IRQ sur les deux PICs
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);

    outb(0x22, 0x70);
    outb(0x23, 0x01);
}
