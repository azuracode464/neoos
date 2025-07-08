[bits 16]
[org 0x7E00]

stage2_start:
    ; Habilitar A20
    call enable_a20
    
    ; Carregar GDT
    cli
    lgdt [gdt_descriptor]
    
    ; Habilitar modo protegido
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    
    ; Pular para código de 32-bit
    jmp CODE_SEG:protected_mode_start

enable_a20:
    ; Método rápido de ativação do A20
    in al, 0x92
    or al, 2
    out 0x92, al
    ret

[bits 32]
protected_mode_start:
    ; Configurar segmentos de dados
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Configurar pilha
    mov esp, 0x90000
    
    ; Verificar modo longo
    call check_long_mode
    
    ; Configurar paginação
    call setup_paging
    
    ; Habilitar modo longo
    jmp enable_long_mode

check_long_mode:
    ; Verificar suporte a modo longo
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_long_mode

    mov eax, 0x80000001
    cpuid
    test edx, (1 << 29)
    jz .no_long_mode
    ret

.no_long_mode:
    mov esi, no_long_mode_msg
    call print_string_32
    jmp $

setup_paging:
    ; Configurar tabelas de paginação básicas
    ; PML4
    mov eax, pml4_table
    or eax, 0b11 ; Presente + Gravável
    mov [pml4_table + 0], eax
    
    ; PDP
    mov eax, pdpt_table
    or eax, 0b11
    mov [pdpt_table + 0], eax
    
    ; PD
    mov eax, pd_table
    or eax, 0b11
    mov [pd_table + 0], eax
    
    ; Mapear primeiro GB (512 páginas de 2MB)
    mov ecx, 0
    mov eax, 0x83 ; Presente + Gravável + Tamanho de página
.map_pd_table:
    mov [pd_table + ecx * 8], eax
    add eax, 0x200000
    inc ecx
    cmp ecx, 512
    jne .map_pd_table
    
    ; Habilitar PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax
    
    ; Carregar CR3 com endereço da PML4
    mov eax, pml4_table
    mov cr3, eax
    
    ret

enable_long_mode:
    ; Habilitar modo longo (EFER.LME=1)
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr
    
    ; Habilitar paginação
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax
    
    ; Carregar GDT de 64-bit
    lgdt [gdt64_descriptor]
    
    ; Atualizar segmentos
    mov ax, GDT64_DATA
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Pular para código de 64-bit
    jmp GDT64_CODE:long_mode_start

[bits 64]
long_mode_start:
    ; Configurar pilha
    mov rsp, 0x7FFFF
    
    ; Limpar tela
    mov edi, 0xB8000
    mov rax, 0x1F201F201F201F20 ; Fundo azul, texto branco, espaço
    mov ecx, 500
    rep stosq
    
    ; Mensagem de boot
    mov rsi, boot_msg
    call print_string_64
    
    ; Carregar kernel
    call load_kernel
    
    ; Pular para o kernel
    jmp 0x100000

load_kernel:
    ; Carregar kernel do disco (setores 5-64)
    mov rdi, 0x100000 ; Endereço de destino
    mov rcx, 5        ; Setor inicial
    mov rdx, 60       ; Número de setores
    
.read_sector:
    push rcx
    push rdx
    push rdi
    
    ; Converter LBA para CHS
    mov rax, rcx
    xor rdx, rdx
    mov rbx, 63       ; Setores por trilha
    div rbx
    
    ; AX = trilha, DX = setor (0-based)
    mov cx, dx
    inc cx            ; Setor (1-based)
    
    ; Calcular cilindro/cabeçote
    xor rdx, rdx
    mov rbx, 16       ; Cabeçotes por cilindro
    div rbx
    
    ; AX = cilindro, DX = cabeçote
    mov dh, dl        ; DH = cabeçote
    mov ch, al        ; CH = cilindro (low 8 bits)
    shr ax, 8
    and al, 0x0F      ; CL[6:7] = cilindro[8:9]
    or cl, al
    
    ; Ler setor
    mov ah, 0x02      ; Função de leitura
    mov al, 1         ; 1 setor
    mov dl, [boot_drive]
    mov rbx, rdi
    int 0x13
    jc .disk_error
    
    pop rdi
    pop rdx
    pop rcx
    
    add rdi, 512      ; Próximo buffer
    inc rcx           ; Próximo setor
    dec rdx
    jnz .read_sector
    ret

.disk_error:
    mov rsi, disk_error_msg
    call print_string_64
    jmp $

; Funções de impressão
print_string_32:
    mov edi, 0xB8000
    mov ah, 0x0F
.loop:
    lodsb
    test al, al
    jz .done
    stosw
    jmp .loop
.done:
    ret

print_string_64:
    mov rdi, 0xB8000
    mov ah, 0x1F ; Cor: fundo azul, texto branco
.loop:
    lodsb
    test al, al
    jz .done
    stosw
    jmp .loop
.done:
    ret

; Dados
boot_drive db 0
boot_msg db "NeoOS Bootloader - Stage 2", 0
disk_error_msg db "Kernel Load Error!", 0
no_long_mode_msg db "64-bit Not Supported!", 0

; GDT de 32-bit
gdt_start:
    dq 0x0 ; Descritor nulo
gdt_code:
    dw 0xFFFF    ; Limit (0-15)
    dw 0x0       ; Base (0-15)
    db 0x0       ; Base (16-23)
    db 10011010b ; Flags (P=1, DPL=00, S=1, Type=1010)
    db 11001111b ; Flags (G=1, D/B=1, AVL=0, Limit 16-19=1111)
    db 0x0       ; Base (24-31)
gdt_data:
    dw 0xFFFF    ; Limit (0-15)
    dw 0x0       ; Base (0-15)
    db 0x0       ; Base (16-23)
    db 10010010b ; Flags (P=1, DPL=00, S=1, Type=0010)
    db 11001111b ; Flags (G=1, D/B=1, AVL=0, Limit 16-19=1111)
    db 0x0       ; Base (24-31)
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; GDT de 64-bit
gdt64_start:
    dq 0x0 ; Descritor nulo
gdt64_code:
    dw 0xFFFF    ; Limit (0-15)
    dw 0x0       ; Base (0-15)
    db 0x0       ; Base (16-23)
    db 10011010b ; Flags (P=1, DPL=00, S=1, Type=1010)
    db 00100000b ; Flags (G=0, D/B=0, L=1, Limit 16-19=0010)
    db 0x0       ; Base (24-31)
gdt64_data:
    dw 0xFFFF    ; Limit (0-15)
    dw 0x0       ; Base (0-15)
    db 0x0       ; Base (16-23)
    db 10010010b ; Flags (P=1, DPL=00, S=1, Type=0010)
    db 00000000b ; Flags (G=0, D/B=0, L=0, Limit 16-19=0000)
    db 0x0       ; Base (24-31)
gdt64_end:

gdt64_descriptor:
    dw gdt64_end - gdt64_start - 1
    dd gdt64_start

GDT64_CODE equ gdt64_code - gdt64_start
GDT64_DATA equ gdt64_data - gdt64_start

; Tabelas de paginação (alinhadas em 4KB)
align 4096
pml4_table:
    times 512 dq 0
pdpt_table:
    times 512 dq 0
pd_table:
    times 512 dq 0

; Preencher até o final do setor
times 16384 - ($ - $$) db 0   ; 16KB
