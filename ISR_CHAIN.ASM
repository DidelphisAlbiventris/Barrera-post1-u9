; Johan Barrera - 1152538 - Arquitectura de computadores - U9
; Solo para DOSBox/modo real DOS, no ejecutable Windows.
bits 16
cpu 8086
org 100h
jmp inicio
%define CHAIN_MODE 1
%include "include/ISRCORE.INC"

%include "include/COMUN.INC"
