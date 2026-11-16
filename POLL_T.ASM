; Johan Barrera - 1152538 - Arquitectura de computadores - U9
; Solo para DOSBox/modo real DOS, no ejecutable Windows.
bits 16
cpu 8086
org 100h
jmp inicio
%ifndef MAX_RETRY
%define MAX_RETRY 0FFFFh
%endif
scan db 0
intentos dw MAX_RETRY
exito db 0
mensaje db 'POLL_T: espera acotada por iteraciones, no por segundos.',13,10,'$'
resultado db 'Dato leido (hex): $'
msg_timeout db 'Timeout: sin respuesta del dispositivo',13,10,'$'
inicio:
    push cs
    pop ds
    cld
    call encabezado
    mov dx, mensaje
    call texto_dos
    call mask_irq1
    call flush_raw
    mov cx, MAX_RETRY
    jcxz timeout
poll:
    in al, 64h
    test al, 1
    jz siguiente
    in al, 60h
    test al, 80h
    jnz siguiente
    test al, al
    jz siguiente
    mov [scan], al
    mov byte [exito], 1
    jmp terminar_poll
siguiente:
    loop poll
timeout:
    mov byte [exito], 0
terminar_poll:
cp_poll:
    call restore_imr
    cmp byte [exito], 0
    je mostrar_timeout
    mov dx, resultado
    call texto_dos
    xor ax, ax
    mov al, [scan]
    call hex_word
    call nueva_linea
    jmp salir
mostrar_timeout:
    mov dx, msg_timeout
    call texto_dos
    jmp salir

%include "include/COMUN.INC"
