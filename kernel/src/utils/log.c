#include <utils/log.h>
#include <utils/kprintf.h>
#include <stdarg.h>
#include <stdint.h>
#include <utils/colours.h>

extern volatile uint64_t timer_ticks;

static const char* level_str[] = {
    "DEBUG",
    "INFO ",
    "WARN ",
    "ERROR",
};

static const char* level_color[] = {
    ANSI_BRIGHT_GREEN,
    ANSI_BRIGHT_CYAN,
    ANSI_BRIGHT_YELLOW,
    ANSI_BRIGHT_RED,
};

static void print_timestamp(void) {
    uint64_t ticks = timer_ticks;
    uint64_t secs = ticks / 1000;
    uint64_t ms = ticks % 1000;

    kprintf(ANSI_BRIGHT_BLACK "[");
    kprintf("%u", (uint64_t)secs);
    kprintf(".");
    if (ms < 100) kprintf("0");
    if (ms < 10) kprintf("0");
    kprintf("%u] " ANSI_RESET, (uint64_t)ms);
}

void log_msg(log_level_t level, const char* subsystem, const char* fmt, ...) {
    if (level < LOG_LEVEL) return;

    print_timestamp();

    kprintf("%s[%s]%s ", level_color[level], level_str[level], ANSI_RESET);

    kprintf(ANSI_BOLD "%s" ANSI_RESET ": ", subsystem);

    va_list args;
    va_start(args, fmt);
    kvprintf(fmt, args);
    va_end(args);

    kprintf("\n");
}
