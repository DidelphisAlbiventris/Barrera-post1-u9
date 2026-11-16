; Johan Barrera - 1152538 - Arquitectura de computadores - U9
; Solo para DOSBox/modo real DOS, no ejecutable Windows.
bits 16
cpu 8086
org 100h
jmp inicio
; Sustituye LPT1.ASM: LPT1 es nombre reservado en Windows incluso con extension.
DATA_PORT equ 0378h
STATUS_PORT equ 0379h
CTRL_PORT equ 037Ah
control_original db 0
ultimo_estado db 0
msg_ini db 'LPT1: prueba acotada sobre 0378h-037Ah.',13,10,'$'
msg_ausente db 'Estado FFh: presencia no confirmada; no se envia dato.',13,10,'$'
msg_busy db 'Timeout esperando listo; no se envio dato.',13,10,'$'
msg_ack db 'Dato 41h enviado; ACK bajo observado. Control restaurado.',13,10,'$'
msg_noack db 'Dato 41h enviado; timeout de ACK. Control restaurado.',13,10,'$'
inicio:
    push cs
    pop ds
    cld
    call encabezado
    mov dx, msg_ini
    call texto_dos
    mov dx, STATUS_PORT
    mov cx, 0FFFFh
wait_ready:
    in al, dx                ; puertos > FFh requieren DX
    mov [ultimo_estado], al
    cmp al, 0FFh
    je no_confirmado
    test al, 80h             ; registro bit7=1: no ocupado (BUSY invertido)
    jnz enviar
    loop wait_ready
    mov dx, msg_busy
    call texto_dos
    jmp salir
enviar:
    mov dx, CTRL_PORT
    in al, dx
    mov [control_original], al
    ; Poner strobe inactivo antes del dato: bit0=0 -> pin /STROBE alto.
    and al, 0FEh
    out dx, al
    mov dx, DATA_PORT
    mov al, 41h
    out dx, al
    mov dx, CTRL_PORT
    mov al, [control_original]
    or al, 01h               ; bit0=1 -> pin /STROBE bajo, activo
    out dx, al
    mov cx, 0FFh             ; retardo didactico, NO calibrado en microsegundos
retardo:
    loop retardo
    and al, 0FEh
    out dx, al
    mov dx, STATUS_PORT
    mov cx, 0FFFFh
wait_ack:
    in al, dx
    test al, 40h             ; pin /ACK bajo
    jz ack_observado
    loop wait_ack
    call restaurar_control
    mov dx, msg_noack
    call texto_dos
    jmp salir
ack_observado:
    call restaurar_control
    mov dx, msg_ack
    call texto_dos
    jmp salir
no_confirmado:
    mov dx, msg_ausente
    call texto_dos
    jmp salir
restaurar_control:
    mov dx, CTRL_PORT
    mov al, [control_original]
    out dx, al
    ret

%include "include/COMUN.INC"
