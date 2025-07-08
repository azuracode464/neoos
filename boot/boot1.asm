[bits 16]
[org 0x7C00]

; Constantes
STAGE2_OFFSET equ 0x7E00   ; Endereço de carregamento do stage2
STAGE2_SECTOR equ 1         ; Setor inicial do stage2
STAGE2_SIZE   equ 4         ; Número de setores para o stage2

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    ; Salvar unidade de boot
    mov [boot_drive], dl

    ; Mensagem de inicialização
    mov si, msg_loading
    call print_string

    ; Carregar stage2 do disco
    mov bx, STAGE2_OFFSET
    mov dh, STAGE2_SIZE
    mov dl, [boot_drive]
    call disk_load

    ; Pular para o stage2
    jmp STAGE2_OFFSET

; Função: print_string
; Parâmetros: SI - ponteiro para string
print_string:
    pusha
    mov ah, 0x0E
.loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    popa
    ret

; Função: disk_load
; Parâmetros:
;   DL - drive number
;   DH - número de setores
;   ES:BX - endereço de destino
disk_load:
    pusha
    push dx

    mov ah, 0x02   ; Função de leitura
    mov al, dh     ; Número de setores
    mov ch, 0x00   ; Cilindro 0
    mov dh, 0x00   ; Cabeçote 0
    mov cl, STAGE2_SECTOR ; Setor inicial

    int 0x13
    jc .error      ; Se carry flag set, erro

    pop dx
    cmp al, dh     ; Verificar setores lidos
    jne .error
    popa
    ret

.error:
    mov si, disk_error_msg
    call print_string
    jmp $

; Dados
boot_drive db 0
msg_loading db "Loading NeoOS...", 0
disk_error_msg db " Disk Error!", 0

; Preenchimento e assinatura de boot
times 510 - ($ - $$) db 0
dw 0xAA55
