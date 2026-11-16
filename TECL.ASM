; Johan Barrera - 1152538 - Arquitectura de computadores - U9
; Solo para DOSBox/modo real DOS, no ejecutable Windows.
bits 16
cpu 8086
org 100h
jmp inicio
scan db 0
mensaje db 'TECL: pulse una letra sin Shift; se ignoran releases/prefijos.',13,10,'$'
resultado db 'Scancode make (hex de 4 posiciones): $'
inicio:
    push cs
    pop ds
    cld
    call encabezado
    mov dx, mensaje
    call texto_dos
    call mask_irq1
    call flush_raw
poll:
    in al, 64h
    test al, 01h
    jz poll
    in al, 60h
    test al, 80h           ; omitir break y prefijos E0/E1
    jnz poll
    test al, al
    jz poll
    mov [scan], al
cp_dato:
    call restore_imr       ; antes de cualquier servicio DOS
    mov dx, resultado
    call texto_dos
    xor ax, ax
    mov al, [scan]
    call hex_word
    call nueva_linea
    jmp salir

%include "include/COMUN.INC"
