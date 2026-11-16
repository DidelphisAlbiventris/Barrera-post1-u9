; Servicios compartidos: siempre preservar los registros de entrada.
; Las cadenas didacticas terminan en NUL; no se envian a DOS AH=09h.
encabezado:
    mov dx, identidad
    call texto_dos
    ret
texto_dos:                    ; DS:DX -> cadena terminada en '$'
    push ax
    mov ah, 09h
    int 21h
    pop ax
    ret
texto_cero:                   ; DS:SI -> cadena terminada en 0
    push ax
    push dx
    push si
    cld
.siguiente:
    lodsb
    test al, al
    jz .fin
    mov dl, al
    mov ah, 02h
    int 21h
    jmp .siguiente
.fin:
    pop si
    pop dx
    pop ax
    ret
nueva_linea:
    push dx
    mov dx, crlf
    call texto_dos
    pop dx
    ret
caracter_dos:                 ; DL -> caracter, preserva AX
    push ax
    mov ah, 02h
    int 21h
    pop ax
    ret
imprimirAX:                   ; AX sin signo -> decimal, incluido cero
    push ax
    push bx
    push cx
    push dx
    mov bx, 10
    xor cx, cx
.dividir:
    xor dx, dx
    div bx
    push dx
    inc cx
    test ax, ax
    jnz .dividir
.emitir:
    pop dx
    add dl, '0'
    call caracter_dos
    loop .emitir
    pop dx
    pop cx
    pop bx
    pop ax
    ret
hex_word:                     ; AX -> cuatro digitos, conserva registros
    push ax
    push bx
    push cx
    push dx
    mov bx, ax
    mov cx, 4
.digito:
    rol bx, 1
    rol bx, 1
    rol bx, 1
    rol bx, 1
    mov dl, bl
    and dl, 0Fh
    cmp dl, 9
    jbe .decimal
    add dl, 'A'-10
    jmp .mostrar
.decimal:
    add dl, '0'
.mostrar:
    call caracter_dos
    loop .digito
    pop dx
    pop cx
    pop bx
    pop ax
    ret
bcd_dos:                      ; AL BCD empaquetado valido -> dos digitos
    push ax
    push bx
    push cx
    push dx
    mov bl, al
    mov cl, 4
    shr al, cl
    mov dl, al
    add dl, '0'
    call caracter_dos
    mov dl, bl
    and dl, 0Fh
    add dl, '0'
    call caracter_dos
    pop dx
    pop cx
    pop bx
    pop ax
    ret
mostrar_estado:               ; DS:SI -> instantanea AX,BX,CX,DX,SI,DI,FLAGS
    push ax
    push bx
    push cx
    push dx
    push si
    push di
    mov di, rotulos
    mov cx, 7
.campo:
    mov dl, [di]
    call caracter_dos
    mov dl, [di+1]
    call caracter_dos
    mov dl, '='
    call caracter_dos
    mov ax, [si]
    call hex_word
    mov dl, ' '
    call caracter_dos
    add si, 2
    add di, 2
    loop .campo
    call nueva_linea
    pop di
    pop si
    pop dx
    pop cx
    pop bx
    pop ax
    ret
pausa_salir:
    mov dx, pausa
    call texto_dos
    mov ah, 07h
    int 21h
salir:
    mov ax, 4C00h
    int 21h
identidad db 'Johan Barrera | Cod: 1152538',13,10
          db 'Arquitectura de computadores | Unidad 9',13,10,13,10,'$'
crlf db 13,10,'$'
pausa db 13,10,'Pulse una tecla para salir.',13,10,'$'
rotulos db 'AXBXCXDXSIDIFL'

; Guardar/enmascarar IRQ1: el BIOS no competira por la lectura de 60h.
mask_irq1:
    pushf
    cli
    in al, 21h
    mov [imr_original], al
    or al, 02h
    out 21h, al
    popf
    ret
restore_imr:
    pushf
    cli
    mov al, [imr_original]
    out 21h, al
    popf
    ret
; Vaciar hasta 32 bytes residuales; nunca un bucle de vaciado infinito.
flush_raw:
    mov cx, 32
.loop:
    in al, 64h
    test al, 1
    jz .done
    in al, 60h
    loop .loop
.done:
    ret
; DX:AX = ticks BDA, lectura atomica; conserva estado IF previo.
read_ticks:
    pushf
    cli
    push es
    mov ax, 40h
    mov es, ax
    mov ax, [es:6Ch]
    mov dx, [es:6Eh]
    pop es
    popf
    ret
; Diferencia respecto a inicio_ticks; soporta un rollover diario del BIOS.
elapsed:
    sub ax, [inicio_ticks]
    sbb dx, [inicio_ticks+2]
    jnc .done
    add ax, 00B0h
    adc dx, 0018h           ; dia BIOS = 001800B0h ticks
.done:
    ret
imr_original db 0
inicio_ticks dd 0
