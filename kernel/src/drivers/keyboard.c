#include <drivers/keyboard.h>
#include <arch/x86_64/io.h>
#include <kernel/kprintf.h>

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_BUF_SIZE 256

static char keyboard_buffer[KEYBOARD_BUF_SIZE];
static uint8_t buf_head = 0;
static uint8_t buf_tail = 0;

static const char scancode_table[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
    '\b', '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']',
    '\n', 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*',
    0, ' '};

static const char scancode_table_shift[] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',
    '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}',
    '\n', 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*',
    0, ' '};

static uint8_t shift_pressed = 0;
static uint8_t caps_lock = 0;

static void buf_push(char c) {
    uint8_t next = (buf_head + 1) % KEYBOARD_BUF_SIZE;
    if (next != buf_tail) {
        keyboard_buffer[buf_head] = c;
        buf_head = next;
    }
}

void keyboard_handler(void) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    if (scancode & 0x80) {
        uint8_t released = scancode & 0x7F;
        if (released == 0x2A || released == 0x36) shift_pressed = 0;
        return;
    }

    if (scancode == 0x2A || scancode == 0x36) {  // shift
        shift_pressed = 1;
        return;
    }
    if (scancode == 0x3A) {
        caps_lock = !caps_lock;
        return;
    }

    if (scancode < sizeof(scancode_table)) {
        char c = 0;
        uint8_t use_upper = shift_pressed ^ caps_lock;
        if (use_upper)
            c = scancode_table_shift[scancode];
        else
            c = scancode_table[scancode];

        if (c) buf_push(c);
    }
}

char keyboard_getchar(void) {
    while (buf_head == buf_tail) {
        __asm__ volatile("hlt");
    }
    char c = keyboard_buffer[buf_tail];
    buf_tail = (buf_tail + 1) % KEYBOARD_BUF_SIZE;
    return c;
}

void keyboard_init(void) {
}
