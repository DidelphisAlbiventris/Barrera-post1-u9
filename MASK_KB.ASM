; Johan Barrera - 1152538 - Arquitectura de computadores - U9
; Solo para DOSBox/modo real DOS, no ejecutable Windows.
bits 16
cpu 8086
org 100h
jmp inicio
msg_mask db 'IRQ1 enmascarado durante 55 ticks (~3 s).',13,10,'$'
msg_fin db 'IRQ1 restaurado: mascara original recuperada.',13,10,'$'
msg_duracion db 'Ticks transcurridos: $'
duracion dw 0
inicio:
    push cs
    pop ds
    cld
    call encabezado
    mov dx, msg_mask
    call texto_dos
    call mask_irq1
    sti                     ; IRQ0 debe seguir habilitada en el PIC
    call ticks_bios
    mov [inicio_ticks], ax
    mov [inicio_ticks+2], dx
esperar:
    call ticks_bios
    call elapsed            ; diferencia de 32 bits; admite medianoche
    test dx, dx
    jnz restaurar
    cmp ax, 55
    jb esperar
restaurar:
    mov [duracion], ax
    call restore_imr
    mov dx, msg_fin
    call texto_dos
    mov dx, msg_duracion
    call texto_dos
    mov ax, [duracion]
    call imprimirAX
    call nueva_linea
    jmp salir
ticks_bios:
    mov ah, 0
    int 1Ah                 ; CX:DX = ticks, AL = indicador de medianoche
    mov ax, dx
    mov dx, cx              ; DX:AX para resta de doble palabra
    ret

%include "include/COMUN.INC"
