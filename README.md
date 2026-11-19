# VesperOS

A bare-metal x86_64 UEFI Micro OS written in C++.

Designed to boot directly via UEFI firmware (`BOOTX64.EFI`) from a USB drive or emulated machine.

---

## Roadmap

- [ ] Minimal UEFI bootloader / entry point (`BOOTX64.EFI`)
- [ ] UEFI Graphics Output Protocol (GOP) linear framebuffer initialization
- [ ] Basic text / font rendering engine
- [ ] Memory map acquisition & `ExitBootServices` transition
- [ ] Bare-metal runtime: GDT, IDT, Paging, and Exception Handling
- [ ] Serial logging & Keyboard input driver
- [ ] Automated CI/CD releases via GitHub Actions

---

## Booting

1. Format a USB drive as **FAT32**.
2. Create the directory `/EFI/BOOT/` on the drive.
3. Place `BOOTX64.EFI` from Releases into `/EFI/BOOT/`.
4. Boot your PC and select the USB drive from the UEFI Boot Menu.
