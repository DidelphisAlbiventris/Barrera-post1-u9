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

## Parte 2

### Decisión técnica 3 — EOI y contexto de interrupción

El estudiante envía el EOI en las ISR propias después de consumir el dato y registrar la información mínima, antes de restaurar registros e iniciar IRET. Así no 
mantiene innecesariamente el estado de servicio del PIC después de completar su trabajo. Retrasar ese reconocimiento puede bloquear solicitudes de igual o menor 
prioridad en el PIC; enviarlo demasiado pronto puede retirar esa protección cuando todavía se manipula el dispositivo, especialmente si después se permiten }
interrupciones anidadas.

Deben corregirse dos afirmaciones del enunciado: con IF=0, enviar EOI no habilita por sí solo que la CPU atienda otras IRQ enmascarables; y en la prioridad fija 
habitual IRQ0 tiene mayor prioridad que IRQ1, por lo que no necesita el EOI de IRQ1 para superar esa prioridad del PIC, aunque sí necesita IF habilitado en la CPU. No 
se recomienda resolver una ISR larga imprimiendo con INT 21h después de un EOI temprano. DOS no es generalmente reentrante y la impresión sigue prolongando el contexto 
de interrupción.

La solución utilizada mantiene la ISR corta y difiere la presentación al programa principal. En el caso encadenado, el handler BIOS es responsable del EOI y la rutina 
propia evita duplicarlo. El orden correcto depende de quién completa realmente el servicio y de cuándo se permiten nuevas interrupciones, no de una regla universal de 
enviar EOI antes de cualquier impresión.

En los ejemplos originales, ISR_KB posterga el EOI hasta después de imprimir, mientras ISR_TIME libera antes el estado de servicio del PIC tras calcular el intervalo. 
Sin STI y con IF=0 durante ambos recorridos, adelantar únicamente el EOI no reduce por sí solo la espera de la CPU hasta IRET. La recomendación completa consiste en 
terminar pronto el trabajo indispensable, enviar el EOI cuando corresponde y retornar; la impresión se ejecuta después en el flujo principal.

### Decisión técnica 4 — Polling frente a interrupciones

El estudiante reserva el polling continuo para una demostración simple o una espera muy corta en la que bloquear temporalmente al programa sea aceptable. Un bucle que 
espera durante minutos a un teclado es inadecuado si también debe animar la pantalla, atender otro periférico o dejar tiempo útil a otras tareas. Las interrupciones 
permiten que el dispositivo solicite atención y que el programa haga otro trabajo mientras tanto.

Esa ventaja no elimina automáticamente el consumo del programa principal: un programa que instala un ISR y después gira sin descanso sobre un contador sigue realizando 
busy-wait. Por eso las versiones entregadas utilizan HLT mientras esperan y revisan los eventos al despertar. Tampoco se afirma que una interrupción tenga siempre 
menor latencia que el polling.

Un sondeo muy frecuente puede detectar un estado antes que una interrupción retrasada por IF, máscaras, prioridades o costo de entrada al handler. La respuesta por IRQ 
depende de esos factores, mientras la de polling depende del intervalo entre consultas. Para un teclado esporádico, las interrupciones suelen ofrecer un equilibrio 
mejor entre atención y uso de CPU; para sondeos breves o requisitos concretos de latencia, puede justificarse otra estrategia.

La comparación debe hacerse con el diseño completo y no solo con la presencia de IN o IRET.

Si el sondeo se programa cada T unidades de tiempo, un evento puede esperar casi T antes de detectarse; con llegadas distribuidas uniformemente, la espera media se 
aproxima a T/2, sin contar el procesamiento. En una solución por interrupciones la demora incluye el bloqueo temporal de IRQ y la entrada a la rutina. Por ello se 
comparan eficiencia y latencia por separado.
