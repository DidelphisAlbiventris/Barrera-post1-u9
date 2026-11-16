MAX_KEYS equ 5
TIMEOUT_TICKS equ 546       ; aproximadamente 30 segundos
old_isr dd 0
contador dw 0
mostradas dw 0
scans times MAX_KEYS db 0
deltas times MAX_KEYS dd 0
ticks_prev dd 0
timeout_ocurrido db 0
msg_ini db 'Pulse cinco letras simples, una por una; sin mantenerlas.',13,10
        db 'Espera maxima ~30 s. Impresion fuera de la ISR.',13,10,'$'
msg_tecla db 'Tecla detectada por ISR propio$'
msg_scan db ' ; scan=$'
msg_ticks db 'Ticks desde evento make anterior: $'
msg_eco db 'Eco BIOS/DOS: $'
msg_fin db 'ISR restaurado. Mascara PIC restaurada. Fin del programa.',13,10,'$'
msg_to db 'Tiempo de espera agotado: se restaura sin completar cinco.',13,10,'$'

inicio:
    push cs
    pop ds
    cld
    call encabezado
    mov dx, msg_ini
    call texto_dos
    ; Obtener vector original y conservar el IMR completo.
    mov ax, 3509h
    int 21h
    mov [old_isr], bx
    mov [old_isr+2], es
    call mask_irq1
%ifndef CHAIN_MODE
    call flush_raw
%endif
    call read_ticks
    mov [inicio_ticks], ax
    mov [inicio_ticks+2], dx
    mov [ticks_prev], ax
    mov [ticks_prev+2], dx
    mov dx, mi_isr
    mov ax, 2509h
    int 21h
    pushf
    cli
    mov al, [imr_original]
    and al, 0FDh
    out 21h, al
    popf
    sti
principal:
    ; Evento publicado por contador despues de llenar su casilla.
    mov bx, [mostradas]
    cmp bx, [contador]
    jb presentar
    cmp bx, MAX_KEYS
    jae terminar
    call read_ticks
    call elapsed
    test dx, dx
    jnz expiro
    cmp ax, TIMEOUT_TICKS
    jae expiro
    ; CLI/check/STI/HLT evita la carrera entre comprobar y dormir.
    cli
    mov bx, [mostradas]
    cmp bx, [contador]
    jb despertar
    sti
    hlt
    jmp principal
despertar:
    sti
    jmp principal
presentar:
%ifdef CHAIN_MODE
    ; Consumir del buffer BIOS en primer plano y mostrar con DOS.
    mov ah, 01h
    int 16h
    jz principal
    xor ah, ah
    int 16h
    push ax
    mov dx, msg_eco
    call texto_dos
    pop ax
    mov dl, al
    call caracter_dos
    call nueva_linea
%else
%ifdef TIME_MODE
    mov dx, msg_ticks
    call texto_dos
    mov bx, [mostradas]
    shl bx, 1
    shl bx, 1
    ; Mostrar numero decimal completo para intervalos <65536 ticks.
    mov ax, [deltas+bx]
    call imprimirAX
    call nueva_linea
%else
    mov dx, msg_tecla
    call texto_dos
    mov dx, msg_scan
    call texto_dos
    mov bx, [mostradas]
    xor ax, ax
    mov al, [scans+bx]
    call hex_word
    call nueva_linea
%endif
%endif
    inc word [mostradas]
    jmp principal
expiro:
    mov byte [timeout_ocurrido], 1
terminar:
    call desinstalar
    cmp byte [timeout_ocurrido], 0
    je imprimir_fin
    mov dx, msg_to
    call texto_dos
imprimir_fin:
    mov dx, msg_fin
    call texto_dos
    jmp salir

desinstalar:
    pushf
    cli
    in al, 21h
    or al, 02h
    out 21h, al             ; proteger ventana de restauracion
    push ds
    lds dx, [old_isr]
    mov ax, 2509h
    int 21h
    pop ds                 ; DS vuelve al programa antes de usar sus datos
    mov al, [imr_original]
    out 21h, al
    popf
    ret

mi_isr:
%ifdef CHAIN_MODE
    ; No leer 60h: pertenece al handler BIOS que se va a encadenar.
    push bp
    mov bp, sp
    sub sp, 2              ; dato local por invocacion, no compartido
    push ax
    push es
    mov ax, 40h
    mov es, ax
    mov ax, [es:1Ch]
    mov [ss:bp-2], ax
    pop es
    pop ax
    pushf
    call far [cs:old_isr]   ; BIOS consume dato, actualiza buffer y envia EOI
    push ax
    push bx
    push ds
    push es
    mov ax, cs
    mov ds, ax
    mov ax, 40h
    mov es, ax
    mov bx, [ss:bp-2]
    cmp bx, [es:1Ch]
    je fin_chain
    cmp word [contador], MAX_KEYS
    jae fin_chain
    inc word [contador]    ; cuenta solo nuevas entradas BIOS, no break codes
fin_chain:
    pop es
    pop ds
    pop bx
    pop ax
    mov sp, bp
    pop bp
    iret                   ; NO segundo EOI
%else
    push ax
    push bx
    push cx
    push dx
    push si
    push di
    push ds
    push es
    mov ax, cs
    mov ds, ax
    in al, 60h             ; libera el buffer de salida del 8042
    test al, 80h
    jnz eoi                ; omitir break y prefijos (alcance: letras simples)
    test al, al
    jz eoi
    mov bx, [contador]
    cmp bx, MAX_KEYS
    jae eoi
    mov [scans+bx], al
%ifdef TIME_MODE
    mov ax, 40h
    mov es, ax
    mov ax, [es:6Ch]
    mov dx, [es:6Eh]        ; IRQ0 no interrumpe esta lectura (IF=0)
    mov si, ax
    mov di, dx
    sub ax, [ticks_prev]
    sbb dx, [ticks_prev+2]
    jnc delta_listo
    add ax, 00B0h
    adc dx, 0018h          ; corregir el reinicio diario del contador BIOS
delta_listo:
    mov [ticks_prev], si
    mov [ticks_prev+2], di
    shl bx, 1
    shl bx, 1
    mov [deltas+bx], ax
    mov [deltas+bx+2], dx
%endif
    inc word [contador]    ; publicar despues de guardar todos los datos
eoi:
    mov al, 20h
    out 20h, al            ; un EOI por IRQ atendida, incluso break/prefijo
    pop es
    pop ds
    pop di
    pop si
    pop dx
    pop cx
    pop bx
    pop ax
    iret
%endif
