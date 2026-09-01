/**
 * generate_boot_sector.js
 * Genera un sector de arranque de 2048 bytes (El Torito No-Emulation) para Mini Windows XP.
 *
 * CORRECCIONES CRÍTICAS vs. versiones anteriores:
 * 1. DS se inicializa a 0x07C0 (NO push cs/pop ds, porque SeaBIOS salta con CS=0x0000)
 * 2. Todos los saltos relativos (jmp/jz/jnc/loop) calculados SIN p++ para evitar off-by-one
 * 3. CLD antes de LODSB para garantizar dirección de lectura de memoria hacia adelante
 * 4. Progreso visual con "." por cada bloque leído
 *
 * Carga XP.BIN (370 KB NTLDR) en 0x2000:0000 y salta a él.
 */

function createMiniXpBootSector(xpBinLba2048) {
    const buf = Buffer.alloc(2048, 0);
    let p = 0;

    // Helper: escribe bytes sin ambigüedad de p++
    function emit(...bytes) {
        for (const b of bytes) {
            buf[p++] = b & 0xFF;
        }
    }

    // Helper: escribe salto relativo hacia adelante (devuelve posición para fixup)
    function emitJccForward(opcode) {
        const pos = p;
        emit(opcode, 0x00);
        return pos;
    }

    // Helper: fixup de salto relativo hacia adelante
    function fixupForward(instrPos) {
        buf[instrPos + 1] = (p - (instrPos + 2)) & 0xFF;
    }

    // Helper: escribe salto relativo hacia atrás
    function emitJmpBack(opcode, target) {
        const instrStart = p;
        emit(opcode, (target - (instrStart + 2)) & 0xFF);
    }

    // ========== DATA AREA (offset 0x100..0x1FF) ==========
    const DATA_BASE = 0x100;

    const MSG_BOOT = DATA_BASE;
    buf.write("Booting Mini Windows XP...\r\n\0", MSG_BOOT, 'ascii');

    const MSG_ERR = DATA_BASE + 30;
    buf.write("\r\nDisk read error!\r\n\0", MSG_ERR, 'ascii');

    const MSG_OK = DATA_BASE + 52;
    buf.write(" OK\r\n\0", MSG_OK, 'ascii');

    // DAP (Disk Address Packet) de 16 bytes en offset 0x130
    // SeaBIOS CD-ROM INT 13h usa sectores nativos de 2048 bytes (NO 512).
    const DAP = DATA_BASE + 0x30;
    buf[DAP + 0] = 0x10;       // Tamaño del DAP
    buf[DAP + 1] = 0x00;       // Reservado
    buf.writeUInt16LE(32, DAP + 2);       // 32 sectores × 2048 bytes = 64 KB por chunk
    buf.writeUInt16LE(0x0000, DAP + 4);   // Offset del búfer
    buf.writeUInt16LE(0x2000, DAP + 6);   // Segmento del búfer
    buf.writeUInt32LE(xpBinLba2048, DAP + 8);  // LBA en sectores de 2048 bytes (nativo CD-ROM)
    buf.writeUInt32LE(0, DAP + 12);            // LBA alto

    // Almacenamiento de la unidad de arranque
    const BOOT_DRV = DATA_BASE + 0x40;

    // ========== CÓDIGO (offset 0x0000) ==========

    // --- Inicialización de segmentos ---
    // SeaBIOS El Torito No-Emulation carga el sector en 0x7C00 y salta con CS=0x0000, IP=0x7C00.
    // Necesitamos DS=0x07C0 para que las referencias a datos (mensajes, DAP) apunten correctamente.
    emit(0xFA);                           // cli
    emit(0xB8, 0xC0, 0x07);              // mov ax, 0x07C0
    emit(0x8E, 0xD8);                    // mov ds, ax
    emit(0x8E, 0xC0);                    // mov es, ax
    emit(0x31, 0xC0);                    // xor ax, ax
    emit(0x8E, 0xD0);                    // mov ss, ax
    emit(0xBC, 0x00, 0x7C);              // mov sp, 0x7C00
    emit(0xFB);                           // sti
    emit(0xFC);                           // cld (dirección ascendente para lodsb)

    // Guardar número de unidad BIOS (DL)
    emit(0x88, 0x16, BOOT_DRV & 0xFF, (BOOT_DRV >> 8) & 0xFF);  // mov [BOOT_DRV], dl

    // --- Imprimir mensaje de arranque ---
    emit(0xBE, MSG_BOOT & 0xFF, (MSG_BOOT >> 8) & 0xFF);  // mov si, MSG_BOOT
    const printBootLoop = p;
    emit(0xAC);                           // lodsb
    emit(0x08, 0xC0);                    // or al, al
    const jzAfterBoot = emitJccForward(0x74);  // jz → (después del loop de impresión)
    emit(0xB4, 0x0E);                    // mov ah, 0x0E (teletype output)
    emit(0xBB, 0x07, 0x00);              // mov bx, 0x0007 (page 0, light gray)
    emit(0xCD, 0x10);                    // int 0x10
    emitJmpBack(0xEB, printBootLoop);    // jmp printBootLoop
    fixupForward(jzAfterBoot);

    // --- Bucle principal de lectura de sectores ---
    // XP.BIN = 370268 bytes. En sectores de 2048: ceil(370268/2048) = 181 sectores.
    // 5 bloques × 32 sectores + 1 bloque × 21 sectores = 181 sectores = 370688 bytes.
    emit(0xB9, 0x06, 0x00);              // mov cx, 6 (6 iteraciones)

    const readLoop = p;
    emit(0x51);                           // push cx

    // ¿Último bloque? Si cx==1, leer solo 21 sectores (21 × 2048 = 43008 bytes)
    emit(0x83, 0xF9, 0x01);              // cmp cx, 1
    const jneNotLast = emitJccForward(0x75);  // jne → .notLast
    emit(0xC7, 0x06,                     // mov word [DAP+2], 21
        (DAP + 2) & 0xFF, ((DAP + 2) >> 8) & 0xFF,
        21, 0x00);
    fixupForward(jneNotLast);

    // Preparar y ejecutar INT 13h AH=42h (Extended Read)
    emit(0xBE, DAP & 0xFF, (DAP >> 8) & 0xFF);  // mov si, DAP
    emit(0x8A, 0x16, BOOT_DRV & 0xFF, (BOOT_DRV >> 8) & 0xFF);  // mov dl, [BOOT_DRV]
    emit(0xB4, 0x42);                    // mov ah, 0x42
    emit(0xCD, 0x13);                    // int 0x13
    const jncReadOk = emitJccForward(0x73);  // jnc → .readOk

    // --- Error de lectura ---
    emit(0xBE, MSG_ERR & 0xFF, (MSG_ERR >> 8) & 0xFF);  // mov si, MSG_ERR
    const errPrintLoop = p;
    emit(0xAC);                           // lodsb
    emit(0x08, 0xC0);                    // or al, al
    const jzHalt = emitJccForward(0x74); // jz → .halt
    emit(0xB4, 0x0E);                    // mov ah, 0x0E
    emit(0xBB, 0x07, 0x00);              // mov bx, 0x0007
    emit(0xCD, 0x10);                    // int 0x10
    emitJmpBack(0xEB, errPrintLoop);     // jmp errPrintLoop
    fixupForward(jzHalt);

    // .halt:
    const haltAddr = p;
    emit(0xF4);                           // hlt
    emitJmpBack(0xEB, haltAddr);         // jmp .halt (bucle infinito)

    // --- Lectura exitosa ---
    fixupForward(jncReadOk);

    // Imprimir "." de progreso
    emit(0xB0, 0x2E);                    // mov al, '.'
    emit(0xB4, 0x0E);                    // mov ah, 0x0E
    emit(0xBB, 0x07, 0x00);              // mov bx, 0x0007
    emit(0xCD, 0x10);                    // int 0x10

    // Avanzar segmento destino: += 0x1000 (64 KB)
    emit(0x81, 0x06,                     // add word [DAP+6], 0x1000
        (DAP + 6) & 0xFF, ((DAP + 6) >> 8) & 0xFF,
        0x00, 0x10);

    // Avanzar LBA: += 32 sectores (de 2048 bytes cada uno)
    emit(0x81, 0x06,                     // add word [DAP+8], 32
        (DAP + 8) & 0xFF, ((DAP + 8) >> 8) & 0xFF,
        32, 0x00);
    emit(0x83, 0x16,                     // adc word [DAP+10], 0
        (DAP + 10) & 0xFF, ((DAP + 10) >> 8) & 0xFF,
        0x00);

    // Loop
    emit(0x59);                           // pop cx
    emitJmpBack(0xE2, readLoop);         // loop readLoop

    // --- Carga completa: imprimir " OK" y saltar a NTLDR ---
    emit(0xBE, MSG_OK & 0xFF, (MSG_OK >> 8) & 0xFF);  // mov si, MSG_OK
    const okPrintLoop = p;
    emit(0xAC);                           // lodsb
    emit(0x08, 0xC0);                    // or al, al
    const jzJump = emitJccForward(0x74); // jz → .jump
    emit(0xB4, 0x0E);                    // mov ah, 0x0E
    emit(0xBB, 0x07, 0x00);              // mov bx, 0x0007
    emit(0xCD, 0x10);                    // int 0x10
    emitJmpBack(0xEB, okPrintLoop);      // jmp okPrintLoop
    fixupForward(jzJump);

    // .jump: Restaurar DL y saltar a NTLDR en 0x2000:0x0000
    emit(0x8A, 0x16, BOOT_DRV & 0xFF, (BOOT_DRV >> 8) & 0xFF);  // mov dl, [BOOT_DRV]
    emit(0xEA, 0x00, 0x00, 0x00, 0x20);  // jmp far 0x2000:0x0000

    // --- Firma de arranque ---
    buf[510] = 0x55;
    buf[511] = 0xAA;

    console.log(`[+] Sector de arranque: ${p} bytes de código (máximo ${DATA_BASE} antes del área de datos)`);
    if (p > DATA_BASE) {
        throw new Error(`¡Código (${p} bytes) invade el área de datos (${DATA_BASE})!`);
    }

    return buf;
}

module.exports = { createMiniXpBootSector };
