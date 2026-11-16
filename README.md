# Unidad 9 — Puertos, polling e interrupciones

**Estudiante:** Johan Barrera  
**Código:** 1152538  
**Asignatura:** Arquitectura de computadores  
**Universidad:** UFPS — Cúcuta  
**Repositorio:** `barrera-post1-u9`

## Parte 1

### Decisión técnica 1 — Polling sin límite o timeout

El estudiante acepta el bucle sin límite de TECL únicamente como demostración controlada, con una persona preparada para pulsar una tecla y con la posibilidad de 
reiniciar DOSBox si no llega el evento. No se considera una estrategia general de recuperación frente a fallas. En POLL_T se prefiere acotar las consultas porque el 
dispositivo puede no responder o el programa puede necesitar continuar con otra tarea.

Un límite demasiado pequeño, como cinco intentos, provoca falsos timeouts: el dispositivo puede estar funcionando y la respuesta llegar después de que el programa ya 
dejó de esperar. El valor adecuado depende del comportamiento esperado y del costo de cada consulta. Contar iteraciones no equivale a medir segundos, puesto que los 
ciclos configurados en el emulador y la velocidad de ejecución alteran cuántas consultas caben en un intervalo.

El contador BIOS de ticks ofrece una referencia temporal independiente del número de instrucciones del bucle y permite establecer una espera aproximadamente medible. 
Sin embargo, tiene resolución limitada y depende de que el timer siga atendido; pausar el emulador o impedir IRQ0 afecta esa referencia. Por ello se distingue 
explícitamente entre presupuesto de reintentos y timeout basado en reloj.

El contador de ticks se encuentra en 0040h:006Ch y, en la configuración BIOS habitual, avanza aproximadamente 18,2 veces por segundo. Una implementación temporal debe 
calcular diferencias con el ancho adecuado y considerar el reinicio de medianoche. La espera por puerto 64h también debe coordinarse con IRQ1 para evitar que el BIOS 
consuma el dato antes del programa.

### Decisión técnica 2 — OR, identidad de la fuente y AND

El estudiante combina con OR las banderas cuando basta con atender cualquiera de los dos dispositivos. Si LPT reporta disponibilidad permanentemente, el OR ya es uno 
en la primera consulta y el programa no espera realmente por el teclado. Esa situación reduce la utilidad del mecanismo para detectar actividad nueva del otro 
dispositivo; una aplicación real debería atender el dispositivo listo, retirarlo temporalmente de la espera o buscar una transición relevante, según el protocolo.

El resultado combinado solo responde a la pregunta de si existe alguna condición verdadera. Es necesario conservar y examinar las muestras individuales para 
identificar teclado, LPT o ambos, sin volver a leer estados que podrían haber cambiado. Si se utilizara AND, se exigiría que ambas muestras indicaran disponibilidad, 
útil cuando debe existir una orden del usuario y la impresora debe poder recibir el dato.

Como las lecturas son sucesivas, ni OR ni AND garantizan simultaneidad física exacta; una sincronización más fuerte puede requerir guardar el evento del usuario y 
comprobar de nuevo el periférico antes de actuar. En DOSBox se documenta lo observado sin convertir un puerto no implementado que devuelve FFh en una afirmación de 
impresora presente.

Las banderas se normalizan antes de combinarlas: el bit 0 del estado del teclado y el bit 7 del estado paralelo ocupan posiciones diferentes. BL y BH conservan cada 
muestra; el OR no conserva por sí solo la identidad de la fuente. Además, el orden de lectura impide afirmar cuál evento ocurrió primero físicamente.

