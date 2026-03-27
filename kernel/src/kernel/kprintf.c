#include <flanterm.h>
#include <stdarg.h>
#include <stdint.h>

#include "kprintf.h"

extern struct flanterm_context* ft_ctx;  // déclaré dans main.c

static void kputchar(char c) {
    if (c == '\n') {
        flanterm_write(ft_ctx, "\r\n", 2);
    } else {
        flanterm_write(ft_ctx, &c, 1);
    }
}

static void kputs(const char* s) {
    while (*s) kputchar(*s++);
}

static void kputuint(uint64_t n, uint8_t base) {
    const char* digits = "0123456789ABCDEF";
    char buf[64];
    uint8_t i = 0;
    if (n == 0) {
        kputchar('0');
        return;
    }
    while (n) {
        buf[i++] = digits[n % base];
        n /= base;
    }
    while (i--) kputchar(buf[i]);
}

void kprintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    while (*fmt) {
        if (*fmt != '%') {
            kputchar(*fmt++);
            continue;
        }
        fmt++;
        switch (*fmt++) {
            case 's':
                kputs(va_arg(args, const char*));
                break;
            case 'd': {
                int64_t n = va_arg(args, int64_t);
                if (n < 0) {
                    kputchar('-');
                    n = -n;
                }
                kputuint((uint64_t)n, 10);
                break;
            }
            case 'u':
                kputuint(va_arg(args, uint64_t), 10);
                break;
            case 'x':
                kputuint(va_arg(args, uint64_t), 16);
                break;
            case 'p':
                kputs("0x");
                kputuint((uint64_t)va_arg(args, void*), 16);
                break;
            case 'c':
                kputchar((char)va_arg(args, int));
                break;
            case '%':
                kputchar('%');
                break;
        }
    }

    va_end(args);
}
