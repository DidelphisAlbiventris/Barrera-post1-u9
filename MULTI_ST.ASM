; Johan Barrera - 1152538 - Arquitectura de computadores - U9
; Solo para DOSBox/modo real DOS, no ejecutable Windows.
bits 16
cpu 8086
org 100h
jmp inicio
bandera_k db 0
bandera_l db 0
combinada db 0
msg_ini db 'OR: teclado OBF o estado listo LPT1; limite 65535 iteraciones.',13,10,'$'
msg_k db 'Teclado listo (OBF=1).',13,10,'$'
msg_l db 'LPT1 reporta listo (bit7=1); no prueba presencia fisica.',13,10,'$'
msg_t db 'Timeout: ninguna bandera observada.',13,10,'$'
inicio:
    push cs
    pop ds
    cld
    call encabezado
    mov dx, msg_ini
    call texto_dos
    call mask_irq1
    call flush_raw
    mov si, 0FFFFh
poll:
    in al, 64h
    and al, 1
    mov bl, al
    mov dx, 0379h
    in al, dx
    and al, 80h
    mov cl, 7
    shr al, cl
    mov bh, al
    mov al, bl
    or al, bh
    jnz encontrado
    dec si
    jnz poll
    jmp guardar
encontrado:
    ; Conservar identidad de las dos muestras ANTES de imprimir/restaurar.
guardar:
    mov [bandera_k], bl
    mov [bandera_l], bh
    mov [combinada], al
cp_or:
    call restore_imr
    cmp byte [combinada], 0
    je sin_evento
    cmp byte [bandera_k], 0
    je probar_lpt
    mov dx, msg_k
    call texto_dos
probar_lpt:
    cmp byte [bandera_l], 0
    je fin_multi
    mov dx, msg_l
    call texto_dos
fin_multi:
    jmp salir
sin_evento:
    mov dx, msg_t
    call texto_dos
    jmp salir

%include "include/COMUN.INC"
