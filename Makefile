ASM=nasm
SRC_DIR=src
BUILD_DIR=build

$(BUILD_DIR)/DedOS.img: $(BUILD_DIR)/main.bin
	cp $(BUILD_DIR)/main.bin $(BUILD_DIR)/DedOS.img
	truncate -s 1440k $(BUILD_DIR)/DedOS.img

$(BUILD_DIR)/main.bin: $(SRC_DIR)/main.asm
	$(ASM) $(SRC_DIR)/main.asm -f bin -o $(BUILD_DIR)/main.bin

clean:
	rm $(BUILD_DIR)/*.bin
	rm $(BUILD_DIR)/*.img