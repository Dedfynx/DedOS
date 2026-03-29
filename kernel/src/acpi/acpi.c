#include <utils/log.h>
#include <acpi/acpi.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

extern uint64_t hhdm_offset;

static acpi_rsdt_t* rsdt = NULL;
static acpi_xsdt_t* xsdt = NULL;

static bool acpi_checksum(acpi_sdt_header_t* header) {
    uint8_t sum = 0;
    uint8_t* ptr = (uint8_t*)header;
    for (uint32_t i = 0; i < header->length; i++)
        sum += ptr[i];
    return sum == 0;
}

void acpi_init(void* rsdp_ptr) {
    acpi_rsdp_t* rsdp = (acpi_rsdp_t*)rsdp_ptr;

    if (rsdp->revision >= 2) {
        acpi_xsdp_t* xsdp = (acpi_xsdp_t*)rsdp_ptr;
        xsdt = (acpi_xsdt_t*)(xsdp->xsdt_address + hhdm_offset);
        log_debug("ACPI", "XSDT a %p", xsdt);
    } else {
        rsdt = (acpi_rsdt_t*)((uint64_t)rsdp->rsdt_address + hhdm_offset);
        log_warn("ACPI", "ACPI 1.0, RSDT a %p", rsdt);
    }
}

acpi_sdt_header_t* acpi_find_table(const char* signature) {
    if (xsdt) {
        uint64_t count = (xsdt->header.length - sizeof(acpi_sdt_header_t)) / 8;
        for (uint64_t i = 0; i < count; i++) {
            acpi_sdt_header_t* t = (acpi_sdt_header_t*)xsdt->entries[i];
            if (t->signature[0] == signature[0] && t->signature[1] == signature[1] &&
                t->signature[2] == signature[2] && t->signature[3] == signature[3])
                return t;
        }
    } else if (rsdt) {
        uint64_t count = (rsdt->header.length - sizeof(acpi_sdt_header_t)) / 4;
        for (uint64_t i = 0; i < count; i++) {
            acpi_sdt_header_t* t = (acpi_sdt_header_t*)((uint64_t)rsdt->entries[i] + hhdm_offset);
            if (t->signature[0] == signature[0] && t->signature[1] == signature[1] &&
                t->signature[2] == signature[2] && t->signature[3] == signature[3])
                return t;
        }
    }
    return NULL;
}
