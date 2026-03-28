# DedOS

Little OS Project

## Roadmap

- [x] GDT
- [x] IDT + ISR
- [x] PIC 8259 + IRQ
- [x] PMM (Bitmap Allocator)
- [x] VMM (basique)
- [x] ACPI / MADT
- [x] LAPIC + IOAPIC
- [x] Timer
- [x] Driver Clavier PS/2
- [ ] Scheduler
- [ ] SMP
- [ ] VFS
- [ ] tmpfs
- [ ] Userland + Syscalls
- [ ] PCI
- [ ] Stockage (AHCI/NVMe)

## Dependencies

- [Limine Template](https://codeberg.org/Limine/limine-c-template-x86-64/) - Template de base pour démarrer
- [freestnd-c-hdrs](https://codeberg.org/OSDev/freestnd-c-hdrs-0bsd) - Headers C freestanding
- [cc-runtime](https://codeberg.org/OSDev/cc-runtime) - Runtime C minimal
- [Flanterm](https://codeberg.org/Mintsuki/Flanterm) - Terminal emulator

## Ressources

- [OSDev Wiki](https://osdev.wiki/) - Wiki d'OSDev
- [Intel SDM](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html) - Manuel x86_64

### Books

- [OSTEP](https://pages.cs.wisc.edu/~remzi/OSTEP/) - Operating Systems: Three Easy Pieces (Remzi H. Arpaci-Dusseau and Andrea C. Arpaci-Dusseau)
- [MOS 4rd](https://www.amazon.fr/Modern-Operating-Systems-Andrew-Tanenbaum/dp/013359162X) - Modern Operating Systems, 4rd edition (Andrew S. Tanenbaum)
