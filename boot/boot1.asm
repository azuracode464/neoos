; boot1.asm - Primeiro estágio do bootloader (16-bit)
[bits 16]
[org 0x7C00]

start:
    cli
    ; Configurar segmentos de forma segura
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    ; Limpar tela (usando registradores em vez de valores imediatos)
    mov ax, 0x0600
    mov cx, 0x0000
    mov dx, 0x184F
    mov bh, 0x07
    int 0x10

    ; Posicionar cursor (usando registrador DX)
    mov dx, 0x0000      ; DH=0 (linha), DL=0 (coluna)
    mov bh, 0
    mov ah, 0x02
    int 0x10

    ; Mensagem - use SI corretamente
    mov si, msg_loading
    call print_string

    ; Carregar segundo estágio com endereçamento seguro
    mov bx, 0x7E00      ; ES:BX buffer - usar registrador
    mov es, ax          ; ES=0 (já que AX=0)
    mov si, 3           ; 3 tentativas

.load_retry:
    ; Resetar disco
    xor ah, ah
    mov dl, 0x80
    int 0x13
    jc .load_error

    ; Ler setores
    mov ah, 0x02
    mov al, 4           ; Ler 4 setores (2KB)
    mov ch, 0
    mov cl, 2
    mov dh, 0
    int 0x13
    jnc .load_success

.load_error:
    dec si
    jz disk_error
    jmp .load_retry

.load_success:
    ; Pular para stage2
    jmp 0x0000:0x7E00

; Sub-rotina print_string otimizada
print_string:
    push ax
    push bx
    mov ah, 0x0E
    mov bh, 0
.loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    pop bx
    pop ax
    ret

disk_error:
    mov si, msg_disk_error
    call print_string
    hlt
    jmp $

; Dados devem vir após o código
msg_loading db "Loading NeoOS...", 0x0D, 0x0A, 0
msg_disk_error db "Disk error! Press reset", 0x0D, 0x0A, 0

times 510-($-$$) db 0
dw 0xAA55
