#include <kernel/log.h>
#include <kernel/kprintf.h>
#include <stdarg.h>
#include <stdint.h>

// ANSI colors
#define ANSI_RESET "\033[0m"
#define ANSI_GRAY "\033[90m"
#define ANSI_CYAN "\033[36m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_RED "\033[31m"
#define ANSI_BOLD "\033[1m"

extern volatile uint64_t timer_ticks;

static const char* level_str[] = {
    "DEBUG",
    "INFO ",
    "WARN ",
    "ERROR",
};

static const char* level_color[] = {
    ANSI_GRAY,
    ANSI_CYAN,
    ANSI_YELLOW,
    ANSI_RED,
};

static void print_timestamp(void) {
    uint64_t ticks = timer_ticks;
    uint64_t secs = ticks / 1000;
    uint64_t ms = ticks % 1000;

    kprintf(ANSI_GRAY "[");
    kprintf("%u", (uint64_t)secs);
    kprintf(".");
    if (ms < 100) kprintf("0");
    if (ms < 10) kprintf("0");
    kprintf("%u] " ANSI_RESET, (uint64_t)ms);
}

void log_msg(log_level_t level, const char* subsystem, const char* fmt, ...) {
    if (level < LOG_LEVEL) return;

    print_timestamp();

    kprintf("%s%s%s ", level_color[level], level_str[level], ANSI_RESET);

    kprintf(ANSI_BOLD "%s" ANSI_RESET ": ", subsystem);

    va_list args;
    va_start(args, fmt);
    kvprintf(fmt, args);
    va_end(args);

    kprintf("\n");
}
