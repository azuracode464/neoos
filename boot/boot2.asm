; boot2.asm - Second stage bootloader (64-bit ready)
[BITS 16]
[ORG 0x7E00]

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00           ; Stack grows downward from 0x7C00

    ; Print loading message
    mov si, msg_loading_kernel
    call print_string

    ; Load kernel (16 sectors = 8KB)
    mov bx, 0x8000           ; ES:BX = 0x0000:0x8000
    mov es, ax
    mov ah, 0x02             ; Read function
    mov al, 16               ; Sectors to read
    mov ch, 0                ; Cylinder 0
    mov cl, 3                ; Sector 3
    mov dh, 0                ; Head 0
    mov dl, 0x80             ; Drive 0x80
    int 0x13
    jc disk_error

    ; Check for long mode support
    call check_long_mode
    jc no_long_mode

    ; Set up paging
    call setup_paging

    ; Enter long mode and jump to kernel
    call enter_long_mode

disk_error:
    mov si, msg_disk_error
    call print_string
    hlt
    jmp $

no_long_mode:
    mov si, msg_no_long_mode
    call print_string
    hlt
    jmp $

; --------------------------------------------------
; Utility Functions
; --------------------------------------------------
print_string:
    push ax
    push bx
    mov ah, 0x0E            ; BIOS teletype
    mov bh, 0               ; Page 0
.loop:
    lodsb                   ; Load next char
    test al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    pop bx
    pop ax
    ret

; --------------------------------------------------
; Check for Long Mode Support
; --------------------------------------------------
check_long_mode:
    ; Check for CPUID
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 0x200000
    push eax
    popfd
    pushfd
    pop eax
    xor eax, ecx
    shr eax, 21
    and eax, 1
    push ecx
    popfd
    test eax, eax
    jz .no_support

    ; Check for extended functions
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_support

    ; Check for long mode bit
    mov eax, 0x80000001
    cpuid
    test edx, (1 << 29)
    jz .no_support

    clc                     ; Clear carry = success
    ret
.no_support:
    stc                     ; Set carry = failure
    ret

; --------------------------------------------------
; Set Up Identity Paging (1GB)
; --------------------------------------------------
setup_paging:
    ; Clear memory for page tables (0x1000-0x4000)
    mov edi, 0x1000
    mov ecx, 0x0C00         ; Clear 3 pages (0x3000 bytes)
    xor eax, eax
    rep stosd

    ; PML4 at 0x1000
    mov edi, 0x1000
    mov dword [edi], 0x2000 | 0x03  ; Point to PDP, present + writable
    mov dword [edi+4], 0

    ; PDP at 0x2000
    mov edi, 0x2000
    mov dword [edi], 0x3000 | 0x03  ; Point to PD, present + writable
    mov dword [edi+4], 0

    ; PD at 0x3000 (1GB identity mapped)
    mov edi, 0x3000
    mov dword [edi], 0x00000083      ; 1GB page, present + writable + huge
    mov dword [edi+4], 0

    ret

; --------------------------------------------------
; Switch to Long Mode
; --------------------------------------------------
enter_long_mode:
    ; Disable interrupts
    cli

    ; Enable PAE
    mov eax, cr4
    or eax, (1 << 5)        ; PAE bit
    mov cr4, eax

    ; Set LME bit in EFER MSR
    mov ecx, 0xC0000080
    rdmsr
    or eax, (1 << 8)        ; LME bit
    wrmsr

    ; Enable paging
    mov eax, cr0
    or eax, (1 << 31)       ; PG bit
    mov cr0, eax

    ; Load 64-bit GDT
    lgdt [gdt64_ptr]

    ; Far jump to 64-bit code
    jmp 0x08:long_mode_entry

; --------------------------------------------------
; 64-bit Code Segment
; --------------------------------------------------
[BITS 64]
long_mode_entry:
    ; Clear segment registers
    mov ax, 0x10            ; Data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Jump to kernel (loaded at 0x8000)
    jmp 0x8000

; --------------------------------------------------
; 64-bit GDT
; --------------------------------------------------
gdt64:
    dq 0                    ; Null descriptor
    dq 0x00AF9A000000FFFF   ; Code: exec/read, present, 64-bit
    dq 0x00AF92000000FFFF   ; Data: read/write, present
gdt64_ptr:
    dw gdt64_end - gdt64 - 1
    dq gdt64
gdt64_end:

; --------------------------------------------------
; Messages
; --------------------------------------------------
msg_loading_kernel db "Loading NeoOS kernel...", 0x0D, 0x0A, 0
msg_disk_error db "Error: Disk read failed!", 0x0D, 0x0A, 0
msg_no_long_mode db "Error: 64-bit mode not supported!", 0x0D, 0x0A, 0

; --------------------------------------------------
; Sector Padding
; --------------------------------------------------
times 512-($-$$) db 0
