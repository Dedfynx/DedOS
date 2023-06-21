org 0x7C00
bits 16

%define ENDL 0x0D, 0x0A

;FAT 12 HEADER
jmp short start
nop

bpb_oem:                    db 'MSWIN4.1'           
bpb_bytes_per_sector:       dw 512
bpb_sectors_per_cluster:    db 1
bpb_reserved_sectors:       dw 1
bpb_fat_count:              db 2
bpb_dir_entries_count:      dw 0E0h
bpb_total_sectors:          dw 2880                 
bpb_media_descriptor_type:  db 0F0h                 
bpb_sectors_per_fat:        dw 9                    
bpb_sectors_per_track:      dw 18
bpb_heads:                  dw 2
bpb_hidden_sectors:         dd 0
bpb_large_sector_count:     dd 0      

;Extended boot record
ebr_drive_number:           db 0                    ; 0x00 floppy, 0x80 hdd, useless
                            db 0                    ; reserved
ebr_signature:              db 29h
ebr_volume_id:              db 12h, 34h, 56h, 78h   ; serial number, value doesn't matter
ebr_volume_label:           db 'DEDOS      '        ; 11 bytes, padded with spaces
ebr_system_id:              db 'FAT12   '           ; 8 bytes


start:
    jmp main

puts:
    push si 
    push ax 
.loop:
    lodsb
    or al, al
    jz .done 

    mov ah, 0x0E
    mov bh, 0
    int 0x10
    jmp .loop

.done: 
    pop ax
    pop si
    ret 
       

main:

    ;setup data segments
    mov ax, 0
    mov ds, ax
    mov es, ax

    ;setup stack
    mov ss, ax
    mov sp, 0x7C00

    ;test read
    mov [ebr_drive_number], dl
    mov ax, 1
    mov cl,1
    mov bx, 0x7E00
    call diskRead

    ;print
    mov si, helloWorld
    call puts
    cli
    hlt



;Error Management
floppyError:
    mov si, errDiskRead
    call puts
    jmp wait_key_and_reboot

wait_key_and_reboot:
    ;INT 16,0 - Wait for Keypress and Read Character
    mov ah, 0
    int 16h             
    jmp 0FFFFh:0        ;reboot

.halt:
    cli
    hlt

;Floppy Disk Management

;LBA to CHS conversion
;lba2chs(lbaAddr ax) -> cx[0..5]: sector, cx[6..15]: cylinder, dh: heads
;
;S = ( A % NS ) + 1
;H = (A / NS) % NH
;C = (A / NS) ÷ NH
;
lba2chs:
    ;save
    push ax
    push dx
    ;
    xor dx,dx
    div word [bpb_sectors_per_track]    ;ax = A/NS, dx=A%NS
    inc dx                              ;dx = S = ( A % NS ) + 1
    mov cx, dx                          

    xor dx,dx
    div word [bpb_heads]                ;ax = C, dx=H
    mov dh, dl
    mov ch, al 
    shl ah, 6
    or cl, ah
    ;restore
    pop ax
    mov dl, al                          ; restore only half, the other is H
    pop ax
    ret

;Read fromdisk
; Parameters:
;   - ax: LBA address
;   - cl: number of sectors to read (up to 128)
;   - dl: drive number
;   - es:bx: memory address where to store read data
;
diskRead:
    ;save
    push ax
    push bx
    push cx
    push dx
    push di
    ;
    push cx
    call lba2chs
    pop ax
    ;INT 13,2 Read Disk Sectors call
    ;AH = 02
	;AL = number of sectors to read	(1-128 dec.)
	;CH = track/cylinder number  (0-1023 dec., see below)
	;CL = sector number  (1-17 dec.)
	;DH = head number  (0-15 dec.)
	;DL = drive number (0=A:, 1=2nd floppy, 80h=drive 0, 81h=drive 1)
	;ES:BX = pointer to buffer
    ;
    ;
    ;BIOS disk reads should be retried at least three times and the
	;controller should be reset upon error detection
    ;
    ;
    mov ah, 02h
    mov di, 3
.loop:
    pusha 
    stc
    int 13h
    jnc .done

    ;read failed
    popa
    call diskReset
    dec di
    test di, di
    jnz .loop
.fail:
    jmp floppyError
.done:
    popa
    ;restore
    pop di
    pop dx
    pop cx
    pop bx
    pop ax
    ;
    ret

diskReset:
    pusha
    mov ah,0
    stc
    int 13h 
    jc floppyError
    popa 
    ret
    
    




helloWorld:     db 'Hello World',ENDL, 0
errDiskRead:    db 'Disk Read failed',ENDL, 0

times 510-($-$$) db 0   ;($-$$) = taille du programme en octets
dw 0AA55h               ;signature de demarage

    