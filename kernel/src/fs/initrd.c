#include <fs/initrd.h>
#include <fs/tmpfs.h>
#include <fs/vfs.h>
#include <lib/string.h>
#include <utils/log.h>
#include <limine.h>

__attribute__((used, section(".limine_requests"))) static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST_ID,
    .revision = 0};

struct tar_header {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
} __attribute__((packed));

static uint64_t octal_to_int(const char* str) {
    uint64_t val = 0;
    while (*str >= '0' && *str <= '7') {
        val = (val << 3) | (*str - '0');
        str++;
    }
    return val;
}

void initrd_init(void) {
    struct limine_module_response* response = module_request.response;
    if (!response || response->module_count == 0) {
        log_warn("INITRD", "Aucun module initrd trouve.");
        return;
    }

    struct limine_file* initrd_file = response->modules[0];
    uint8_t* archive = (uint8_t*)initrd_file->address;
    size_t archive_size = initrd_file->size;
    size_t offset = 0;

    log_info("INITRD", "Chargement de l'archive (%d octets)...", (uint32_t)archive_size);

    while (offset < archive_size) {
        struct tar_header* header = (struct tar_header*)(archive + offset);

        if (header->name[0] == '\0') break;

        if (memcmp(header->magic, "ustar", 5) != 0) {
            log_error("INITRD", "Format d'archive non reconnu (besoin de ustar)");
            break;
        }

        uint64_t size = octal_to_int(header->size);
        uint32_t flags = (header->typeflag == '5') ? VFS_DIRECTORY : VFS_FILE;

        char* name = header->name;

        if (name[0] == '.' && name[1] == '/') {
            name += 2;
        }

        if (name[0] == '\0') {
            offset += 512 + ((size + 511) & ~511);
            continue;
        }

        size_t len = strlen(name);
        if (len > 0 && name[len - 1] == '/') {
            name[len - 1] = '\0';
        }

        void* content = (void*)(archive + offset + 512);
        tmpfs_add_entry(name, content, size, flags);

        offset += 512 + ((size + 511) & ~511);
    }

    log_info("INITRD", "Chargement termine.");
}
