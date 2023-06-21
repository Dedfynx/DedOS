org 0x7C00
bits 16

%define ENDL 0x0D, 0x0A

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

    ;print
    mov si, helloWorld
    call puts
    hlt
    
.halt:
    jmp .halt

helloWorld: db 'Hello World',ENDL, 0

times 510-($-$$) db 0   ;($-$$) = taille du programme en octets
dw 0AA55h               ;signature de demarage

    