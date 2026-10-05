La razón por la cual los juegos tradicionales de NES te reconocen el teclado y la PC Vivaz no se debe a que el hardware que esperan uno y otro es completamente diferente.

1. ¿Por qué los juegos normales sí reconocen tu teclado?
En una consola NES estándar (como la que juega a Super Mario, Contra o Zelda):

La consola solo tiene dos puertos frontales para mandos.
Cada mando solo tiene 8 botones (Arriba, Abajo, Izquierda, Derecha, B, A, Select, Start).
La consola lee estos botones mediante un registro serie de 8 bits en la dirección de memoria $4016.
En el emulador, tu teclado físico simplemente está simulando un joystick de 8 botones:
Flechas $\rightarrow$ D-Pad
Z / X $\rightarrow$ Botones B y A
Enter $\rightarrow$ Start
Para el juego de NES, vos tenés un joystick conectado, no una computadora.
2. ¿Qué es en realidad la "PC Vivaz" (pcvivaz-unif.nes)?
La PC Vivaz (Ordenador Educativo 2000 / Educational Computer 2000) no es un juego de consola, sino el software de un Famiclón educativo (como las computadoras Subor o Micro Genius de los años 90):

Era una placa de Famicom metida dentro de una carcasa de teclado QWERTY completo de 101 teclas, con un ratón serial conectado.
No lee el protocolo de joystick de 8 bits: su sistema operativo (PC Escribiendo, Hoja Mágica, etc.) no pregunta por los botones de un joystick en $4016.
En su lugar, el software envía pulsos de selección (strobe) para escanear una matriz de teclado física de 13 filas por 8 columnas (el protocolo de hardware Subor Keyboard / Famicom Keyboard).
Para el cursor en pantalla, espera un ratón serie de Famicom conectado al puerto de expansión auxiliar.
3. ¿Por qué el emulador en el navegador no la reconoce?
Para que la PC Vivaz funcione, el emulador no debe enviar comandos de gamepad: debe emular el chip del teclado matricial Subor y el ratón serial en el bus del sistema.

Investigando el código fuente de los núcleos compilados en WebAssembly (WASM):

Nestopia (el core que cargaba por defecto):
Aunque el núcleo interno de Nestopia reconoce el cartucho de memoria (UNL-EDU2000), el puente de RetroArch/Libretro de Nestopia nunca programó soporte para teclado Subor ni ratón serial. Solo tiene programados mandos convencionales, Zapper (pistola) y paleta Arkanoid. Cuando la ROM de PC Vivaz le pregunta al emulador si hay una tecla pulsada en la matriz, Nestopia devuelve datos vacíos.
FCEUmm (el core secundario):
FCEUmm en su versión de PC sí tiene el código del teclado Subor (SIFC_SUBORKB), pero:
El binario WASM empaquetado para la web (fceumm_libretro.wasm) no tiene habilitado el puerto de expansión para teclado en el frontend.
La ROM pcvivaz-unif.nes está en formato UNIF (un formato antiguo de volcados rusos/chinos). Al leer UNIF, FCEUmm por defecto deja el puerto de expansión desconectado (SIFC_NONE), mientras que solo lo autodetectaba en ROMs .nes con firma CRC específica.
Por eso, aunque apretes las flechas, las letras o muevas el ratón, la ROM de PC Vivaz está esperando que un hardware de matriz de teclado responda en el puerto de expansión, hardware que el emulador web actualmente no tiene conectado al bus.

4. ¿Cómo se pueden "conectar" para usarla?
Hay dos soluciones según dónde quieras ejecutarla:

A. Si querés usarla ahora mismo en tu PC (100% Funcional con Teclado y Mouse reales):
El emulador de NES más avanzado y preciso del mundo es Mesen (gratuito y portable):

Abrís Mesen.
Cargás pcvivaz-unif.nes.
Vas a Opciones $\rightarrow$ Entrada (Input):
En Puerto de Expansión (Expansion Port), seleccionás Subor Keyboard (o Subor Keyboard + Mouse).
Al volver a la pantalla, tu teclado físico de la PC escribe letra por letra y tu mouse mueve el cursor de la PC Vivaz inmediatamente.
B. Para tenerlo en tu plataforma Web (RetroHub / NES WASM):
Para que la PC Vivaz funcione directo en el navegador web como querés, se requieren dos cosas técnicas:

Convertir la ROM de UNIF a NES 2.0 (Mapper 329):
Configurando el byte 15 de la cabecera NES 2.0 con el valor 0x27 (que le ordena al emulador conectar obligatoriamente Subor Keyboard + Mouse).
Integrar un Core WASM que exponga el teclado Subor:
Un build de Mesen WASM o un build de FCEUmm con el flag de Famicom Expansion Keyboard activado en el bus WebAssembly.