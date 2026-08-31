/**
 * generate_boot_sector.js
 * Genera el sector de arranque de 2048 bytes para Mini Windows XP (No Emulation El Torito).
 * Usa offsets relativos al segmento CS (DS = CS) para garantizar direccionamiento exacto de memoria.
 * Carga XP.BIN (370 KB NTLDR) en el segmento 0x2000:0000 y salta a él.
 */

function createMiniXpBootSector(xpBinLba2048) {
    const buf = Buffer.alloc(2048, 0);

    // Variables / Data Offsets (Relativos al segmento CS, inicio en 0x0000)
    const OFFSET_MSG_BOOT   = 0x0050;
    const OFFSET_MSG_ERR    = 0x0070;
    const OFFSET_DAP        = 0x0090;
    const OFFSET_BOOT_DRIVE = 0x00B0;

    // Escribir Mensajes
    buf.write("Booting Mini Windows XP...\r\n\0", OFFSET_MSG_BOOT, 'ascii');
    buf.write("Disk read error!\r\n\0", OFFSET_MSG_ERR, 'ascii');

    // Escribir DAP por defecto
    const xpBinLba512 = xpBinLba2048 * 4;
    buf[OFFSET_DAP + 0] = 0x10; // DAP size = 16 bytes
    buf[OFFSET_DAP + 1] = 0x00; // Reserved
    buf.writeUInt16LE(128, OFFSET_DAP + 2); // Sectors per chunk = 128
    buf.writeUInt16LE(0x0000, OFFSET_DAP + 4); // Offset = 0x0000
    buf.writeUInt16LE(0x2000, OFFSET_DAP + 6); // Segment = 0x2000
    buf.writeUInt32LE(xpBinLba512, OFFSET_DAP + 8); // LBA start (512-byte sectors)
    buf.writeUInt32LE(0, OFFSET_DAP + 12); // LBA high

    let p = 0;

    // cli
    buf[p++] = 0xFA;
    // push cs; pop ds
    buf[p++] = 0x0E; buf[p++] = 0x1F;
    // push cs; pop es
    buf[p++] = 0x0E; buf[p++] = 0x07;
    // xor ax, ax; mov ss, ax; mov sp, 0x7C00; sti
    buf[p++] = 0x31; buf[p++] = 0xC0;
    buf[p++] = 0x8E; buf[p++] = 0xD0;
    buf[p++] = 0xBC; buf[p++] = 0x00; buf[p++] = 0x7C;
    buf[p++] = 0xFB;

    // mov [OFFSET_BOOT_DRIVE], dl  (Save DL boot drive)
    buf[p++] = 0x88; buf[p++] = 0x16; buf[p++] = (OFFSET_BOOT_DRIVE & 0xFF); buf[p++] = (OFFSET_BOOT_DRIVE >> 8);

    // Print boot message
    // mov si, OFFSET_MSG_BOOT
    buf[p++] = 0xBE; buf[p++] = (OFFSET_MSG_BOOT & 0xFF); buf[p++] = (OFFSET_MSG_BOOT >> 8);
    // .print_boot_loop:
    const printBootLoop = p;
    // lodsb
    buf[p++] = 0xAC;
    // or al, al
    buf[p++] = 0x08; buf[p++] = 0xC0;
    // jz .start_read
    const jzBootIdx = p; buf[p++] = 0x74; buf[p++] = 0x00; // rel8 placeholder
    // mov ah, 0x0E
    buf[p++] = 0xB4; buf[p++] = 0x0E;
    // mov bx, 0x0007
    buf[p++] = 0xBB; buf[p++] = 0x07; buf[p++] = 0x00;
    // int 0x10
    buf[p++] = 0xCD; buf[p++] = 0x10;
    // jmp .print_boot_loop
    buf[p++] = 0xEB; buf[p++] = (printBootLoop - (p + 1)) & 0xFF;

    // Fill jz placeholder
    buf[jzBootIdx + 1] = (p - (jzBootIdx + 2)) & 0xFF;

    // .start_read:
    // mov cx, 6  (6 chunks of 128/84 sectors)
    buf[p++] = 0xB9; buf[p++] = 0x06; buf[p++] = 0x00;

    const readLoop = p;
    // push cx
    buf[p++] = 0x51;
    // cmp cx, 1
    buf[p++] = 0x83; buf[p++] = 0xF9; buf[p++] = 0x01;
    // jne .not_last
    const jneLastIdx = p; buf[p++] = 0x75; buf[p++] = 0x00;
    // mov word [OFFSET_DAP + 2], 84 (Last chunk is 84 sectors)
    buf[p++] = 0xC7; buf[p++] = 0x06; buf[p++] = ((OFFSET_DAP + 2) & 0xFF); buf[p++] = ((OFFSET_DAP + 2) >> 8); buf[p++] = 0x54; buf[p++] = 0x00;

    // .not_last:
    buf[jneLastIdx + 1] = (p - (jneLastIdx + 2)) & 0xFF;

    // mov si, OFFSET_DAP
    buf[p++] = 0xBE; buf[p++] = (OFFSET_DAP & 0xFF); buf[p++] = (OFFSET_DAP >> 8);
    // mov dl, [OFFSET_BOOT_DRIVE]
    buf[p++] = 0x8A; buf[p++] = 0x16; buf[p++] = (OFFSET_BOOT_DRIVE & 0xFF); buf[p++] = (OFFSET_BOOT_DRIVE >> 8);
    // mov ah, 0x42
    buf[p++] = 0xB4; buf[p++] = 0x42;
    // int 0x13
    buf[p++] = 0xCD; buf[p++] = 0x13;
    // jnc .read_ok
    const jncReadIdx = p; buf[p++] = 0x73; buf[p++] = 0x00;

    // Read error -> print error message
    // mov si, OFFSET_MSG_ERR
    buf[p++] = 0xBE; buf[p++] = (OFFSET_MSG_ERR & 0xFF); buf[p++] = (OFFSET_MSG_ERR >> 8);
    const errLoop = p;
    // lodsb
    buf[p++] = 0xAC;
    // or al, al
    buf[p++] = 0x08; buf[p++] = 0xC0;
    // jz .halt
    const jzHaltIdx = p; buf[p++] = 0x74; buf[p++] = 0x00;
    // mov ah, 0x0E
    buf[p++] = 0xB4; buf[p++] = 0x0E;
    // mov bx, 0x0007
    buf[p++] = 0xBB; buf[p++] = 0x07; buf[p++] = 0x00;
    // int 0x10
    buf[p++] = 0xCD; buf[p++] = 0x10;
    // jmp .err_loop
    buf[p++] = 0xEB; buf[p++] = (errLoop - (p + 1)) & 0xFF;

    // .halt:
    buf[jzHaltIdx + 1] = (p - (jzHaltIdx + 2)) & 0xFF;
    const haltLabel = p;
    // hlt
    buf[p++] = 0xF4;
    // jmp .halt
    buf[p++] = 0xEB; buf[p++] = (haltLabel - (p + 1)) & 0xFF;

    // .read_ok:
    buf[jncReadIdx + 1] = (p - (jncReadIdx + 2)) & 0xFF;

    // add word [OFFSET_DAP + 6], 0x1000 (Advance segment by 0x1000 = 64KB)
    buf[p++] = 0x81; buf[p++] = 0x06; buf[p++] = ((OFFSET_DAP + 6) & 0xFF); buf[p++] = ((OFFSET_DAP + 6) >> 8); buf[p++] = 0x00; buf[p++] = 0x10;
    // add dword [OFFSET_DAP + 8], 128 (Advance LBA by 128 sectors)
    buf[p++] = 0x81; buf[p++] = 0x06; buf[p++] = ((OFFSET_DAP + 8) & 0xFF); buf[p++] = ((OFFSET_DAP + 8) >> 8); buf[p++] = 0x80; buf[p++] = 0x00;
    buf[p++] = 0x83; buf[p++] = 0x16; buf[p++] = ((OFFSET_DAP + 10) & 0xFF); buf[p++] = ((OFFSET_DAP + 10) >> 8); buf[p++] = 0x00;

    // pop cx
    buf[p++] = 0x59;
    // loop .read_loop
    buf[p++] = 0xE2; buf[p++] = (readLoop - (p + 1)) & 0xFF;

    // Finished reading! Now jump to 0x2000:0000
    // mov dl, [OFFSET_BOOT_DRIVE]
    buf[p++] = 0x8A; buf[p++] = 0x16; buf[p++] = (OFFSET_BOOT_DRIVE & 0xFF); buf[p++] = (OFFSET_BOOT_DRIVE >> 8);
    // jmp far 0x2000:0000
    buf[p++] = 0xEA;
    buf[p++] = 0x00; buf[p++] = 0x00; // offset 0x0000
    buf[p++] = 0x00; buf[p++] = 0x20; // segment 0x2000

    // Magic signature at end of first 512 bytes
    buf[510] = 0x55;
    buf[511] = 0xAA;

    return buf;
}

module.exports = { createMiniXpBootSector };
