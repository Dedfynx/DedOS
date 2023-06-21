ASM=nasm
SRC_DIR=src
BUILD_DIR=build

.PHONY: clean all kernel iso bootloader always

iso: $(BUILD_DIR)/DedOS.img

$(BUILD_DIR)/DedOS.img: bootloader kernel
	dd if=/dev/zero of=$(BUILD_DIR)/DedOS.img bs=512 count=2880
	mkfs.fat -F 12 -n "NBOS" $(BUILD_DIR)/DedOS.img
	dd if=$(BUILD_DIR)/bootloader.bin of=$(BUILD_DIR)/DedOS.img conv=notrunc
	mcopy -i $(BUILD_DIR)/DedOS.img $(BUILD_DIR)/kernel.bin "::kernel.bin"

bootloader: $(BUILD_DIR)/bootloader.bin

$(BUILD_DIR)/bootloader.bin: always
	$(ASM) $(SRC_DIR)/bootloader/boot.asm -f bin -o $(BUILD_DIR)/bootloader.bin

kernel:$(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: always
	$(ASM) $(SRC_DIR)/kernel/main.asm -f bin -o $(BUILD_DIR)/kernel.bin


always:
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)/*