/**
 * build_minixp_iso.js
 * Generador de ISO-9660 + Joliet con arranque El Torito (ISOLINUX / SeaBIOS)
 * para Mini Windows XP en WebAssembly (v86).
 */

const fs = require('fs');
const path = require('path');

const SECTOR_SIZE = 2048;

function alignSector(offset) {
    return Math.ceil(offset / SECTOR_SIZE) * SECTOR_SIZE;
}

function writeBothEndian16(buf, offset, val) {
    buf.writeUInt16LE(val, offset);
    buf.writeUInt16BE(val, offset + 2);
}

function writeBothEndian32(buf, offset, val) {
    buf.writeUInt32LE(val, offset);
    buf.writeUInt32BE(val, offset + 4);
}

function encodeAsciiPadded(str, length) {
    const buf = Buffer.alloc(length, 0x20); // space padded
    buf.write(str, 0, 'ascii');
    return buf;
}

function encodeJolietPadded(str, length) {
    const buf = Buffer.alloc(length, 0x00);
    // write utf16-be spaces initially if needed or 0
    for (let i = 0; i < length; i += 2) {
        buf.writeUInt16BE(0x0020, i);
    }
    for (let i = 0; i < str.length && i * 2 < length; i++) {
        buf.writeUInt16BE(str.charCodeAt(i), i * 2);
    }
    return buf;
}

function formatIsoDate(d = new Date()) {
    const YYYY = String(d.getUTCFullYear()).padStart(4, '0');
    const MM = String(d.getUTCMonth() + 1).padStart(2, '0');
    const DD = String(d.getUTCDate()).padStart(2, '0');
    const hh = String(d.getUTCHours()).padStart(2, '0');
    const mm = String(d.getUTCMinutes()).padStart(2, '0');
    const ss = String(d.getUTCSeconds()).padStart(2, '0');
    const cs = String(Math.floor(d.getUTCMilliseconds() / 10)).padStart(2, '0');
    const buf = Buffer.alloc(17);
    buf.write(`${YYYY}${MM}${DD}${hh}${mm}${ss}${cs}`, 0, 'ascii');
    buf[16] = 0; // GMT
    return buf;
}

function formatDirDate(d = new Date()) {
    const buf = Buffer.alloc(7);
    buf[0] = d.getUTCFullYear() - 1900;
    buf[1] = d.getUTCMonth() + 1;
    buf[2] = d.getUTCDate();
    buf[3] = d.getUTCHours();
    buf[4] = d.getUTCMinutes();
    buf[5] = d.getUTCSeconds();
    buf[6] = 0; // GMT
    return buf;
}

function createDirectoryRecord(isJoliet, name, sectorLba, dataLength, isDir, date = new Date()) {
    let nameBuf;
    if (name === '\x00') {
        nameBuf = Buffer.from([0x00]); // Current dir (.)
    } else if (name === '\x01') {
        nameBuf = Buffer.from([0x01]); // Parent dir (..)
    } else if (isJoliet) {
        nameBuf = Buffer.alloc(name.length * 2);
        for (let i = 0; i < name.length; i++) {
            nameBuf.writeUInt16BE(name.charCodeAt(i), i * 2);
        }
    } else {
        const isoName = isDir ? name.toUpperCase() : (name.toUpperCase().includes(';') ? name.toUpperCase() : `${name.toUpperCase()};1`);
        nameBuf = Buffer.from(isoName, 'ascii');
    }

    const nameLen = nameBuf.length;
    let recLen = 33 + nameLen;
    if (recLen % 2 !== 0) recLen++; // Must be even

    const buf = Buffer.alloc(recLen, 0);
    buf[0] = recLen;
    buf[1] = 0; // Extended attribute length
    writeBothEndian32(buf, 2, sectorLba);
    writeBothEndian32(buf, 10, dataLength);
    formatDirDate(date).copy(buf, 18);
    buf[25] = isDir ? 0x02 : 0x00; // File flags: 0x02 = directory
    buf[26] = 0; // File unit size
    buf[27] = 0; // Interleave gap
    writeBothEndian16(buf, 28, 1); // Volume sequence number
    buf[32] = nameLen;
    nameBuf.copy(buf, 33);

    return buf;
}

function calculateIsolinuxChecksum(buf) {
    let sum = 0;
    for (let i = 64; i < buf.length; i += 4) {
        const val = buf.readUInt32LE(i);
        sum = (sum + val) >>> 0;
    }
    return sum;
}

function buildIso(sourceDir, outputIsoPath) {
    console.log("==================================================================");
    console.log("[*] Generando Mini Windows XP Live ISO (Boot ISOLINUX + SeaBIOS)");
    console.log(`[*] Origen:  ${sourceDir}`);
    console.log(`[*] Destino: ${outputIsoPath}`);
    console.log("==================================================================");

    // 1. Recolectar todos los archivos
    const entries = [];
    function scan(currentDir, relPath = '') {
        const files = fs.readdirSync(currentDir);
        for (const file of files) {
            const fullPath = path.join(currentDir, file);
            const currentRel = relPath ? `${relPath}/${file}` : file;
            const stat = fs.statSync(fullPath);
            if (stat.isDirectory()) {
                entries.push({
                    isDir: true,
                    relPath: currentRel,
                    fullPath: fullPath,
                    name: file,
                    size: 0
                });
                scan(fullPath, currentRel);
            } else {
                entries.push({
                    isDir: false,
                    relPath: currentRel,
                    fullPath: fullPath,
                    name: file,
                    size: stat.size
                });
            }
        }
    }
    scan(sourceDir);

    // También aseguramos que isolinux.cfg esté accesible en la raíz y en /isolinux/ para máxima compatibilidad
    let hasRootIsolinuxCfg = entries.some(e => e.relPath.toLowerCase() === 'isolinux.cfg');
    const hbcdIsolinuxCfg = entries.find(e => e.relPath.toLowerCase() === 'hbcd/isolinux.cfg');

    // Estructurar árbol de directorios
    // Directorios:
    // / (root)
    // /HBCD
    // /HBCD/BOOT
    // /HBCD/XP
    const dirMap = new Map();
    dirMap.set('', { path: '', children: [] });

    for (const e of entries) {
        const parentPath = path.dirname(e.relPath) === '.' ? '' : path.dirname(e.relPath).replace(/\\/g, '/');
        if (!dirMap.has(parentPath)) {
            dirMap.set(parentPath, { path: parentPath, children: [] });
        }
        dirMap.get(parentPath).children.push(e);
        if (e.isDir && !dirMap.has(e.relPath)) {
            dirMap.set(e.relPath, { path: e.relPath, children: [] });
        }
    }

    // 2. Planificar LBAs
    // Sector 0-15: System area (32KB)
    // Sector 16: Primary Volume Descriptor (PVD)
    // Sector 17: Supplementary Volume Descriptor (Joliet)
    // Sector 18: El Torito Boot Record Descriptor
    // Sector 19: Volume Descriptor Set Terminator
    // Sector 20: El Torito Boot Catalog
    // Sector 21: ISO9660 Path Table L
    // Sector 22: ISO9660 Path Table M
    // Sector 23: Joliet Path Table L
    // Sector 24: Joliet Path Table M
    // Sector 25..: Directory Blocks (ISO & Joliet)
    // Siguiente sector: Files data

    const PVD_LBA = 16;
    const JOLIET_VD_LBA = 17;
    const BOOT_VD_LBA = 18;
    const TERM_VD_LBA = 19;
    const BOOT_CATALOG_LBA = 20;

    let currentLba = 25;

    // Directorios ISO y Joliet
    const dirsList = Array.from(dirMap.keys()).sort((a, b) => a.length - b.length);
    const isoDirLbas = new Map();
    const jolietDirLbas = new Map();

    for (const d of dirsList) {
        isoDirLbas.set(d, currentLba);
        currentLba += 1; // 1 sector por directorio es más que suficiente para nuestra cantidad de archivos
    }
    for (const d of dirsList) {
        jolietDirLbas.set(d, currentLba);
        currentLba += 1;
    }

    // Archivos
    const fileLbas = new Map();
    let isolinuxBinEntry = null;

    for (const e of entries) {
        if (!e.isDir) {
            fileLbas.set(e.relPath, currentLba);
            const sectors = Math.ceil(e.size / SECTOR_SIZE) || 1;
            if (e.relPath.toLowerCase() === 'hbcd/isolinux.bin' || e.relPath.toLowerCase() === 'isolinux.bin') {
                isolinuxBinEntry = { ...e, lba: currentLba, sectors: sectors };
            }
            currentLba += sectors;
        }
    }

    if (!isolinuxBinEntry) {
        throw new Error("No se encontró ISOLINUX.BIN en el origen.");
    }

    const totalSectors = currentLba;
    console.log(`[+] Total Sectores ISO: ${totalSectors} (~${((totalSectors * 2048) / (1024 * 1024)).toFixed(2)} MB)`);
    console.log(`[+] ISOLINUX.BIN LBA: ${isolinuxBinEntry.lba}, Tamaño: ${isolinuxBinEntry.size} bytes`);

    // 3. Crear el archivo ISO
    const fd = fs.openSync(outputIsoPath, 'w');
    const zeroSector = Buffer.alloc(SECTOR_SIZE, 0);

    function writeSectorAt(buf, lba) {
        fs.writeSync(fd, buf, 0, buf.length, lba * SECTOR_SIZE);
    }

    // Rellenar hasta el final con ceros
    fs.writeSync(fd, zeroSector, 0, SECTOR_SIZE, (totalSectors - 1) * SECTOR_SIZE);

    // ==========================================
    // SECTOR 16: Primary Volume Descriptor (ISO9660)
    // ==========================================
    const pvd = Buffer.alloc(SECTOR_SIZE, 0);
    pvd[0] = 0x01; // Type 1: Primary
    pvd.write("CD001", 1, 'ascii');
    pvd[6] = 0x01; // Version
    encodeAsciiPadded("MINIXP", 32).copy(pvd, 40); // Volume identifier
    writeBothEndian32(pvd, 80, totalSectors);
    writeBothEndian16(pvd, 120, 1); // Volume set size
    writeBothEndian16(pvd, 124, 1); // Volume sequence number
    writeBothEndian16(pvd, 128, SECTOR_SIZE); // Logical block size
    writeBothEndian32(pvd, 132, 10); // Path table size
    pvd.writeUInt32LE(21, 140); // Type L path table
    pvd.writeUInt32BE(22, 148); // Type M path table

    // Root directory record en PVD
    const isoRootDirRec = createDirectoryRecord(false, '\x00', isoDirLbas.get(''), SECTOR_SIZE, true);
    isoRootDirRec.copy(pvd, 156);

    encodeAsciiPadded("MINIXP", 128).copy(pvd, 190); // Volume Set Id
    encodeAsciiPadded("DEVILUTIONX", 128).copy(pvd, 318); // Publisher Id
    encodeAsciiPadded("DEVILUTIONX", 128).copy(pvd, 446); // Data Preparer Id
    encodeAsciiPadded("V86_MINIXP_BUILDER", 128).copy(pvd, 574); // Application Id
    formatIsoDate().copy(pvd, 813); // Creation Date
    formatIsoDate().copy(pvd, 830); // Modification Date
    formatIsoDate().copy(pvd, 847); // Expiration Date
    formatIsoDate().copy(pvd, 864); // Effective Date
    pvd[881] = 0x01; // File structure version
    writeSectorAt(pvd, PVD_LBA);

    // ==========================================
    // SECTOR 17: Supplementary Volume Descriptor (Joliet)
    // ==========================================
    const jvd = Buffer.alloc(SECTOR_SIZE, 0);
    jvd[0] = 0x02; // Type 2: Supplementary
    jvd.write("CD001", 1, 'ascii');
    jvd[6] = 0x01; // Version
    jvd[7] = 0x00; // Flags
    encodeAsciiPadded("MINIXP", 32).copy(jvd, 8); // System Id
    encodeJolietPadded("Mini Windows XP", 32).copy(jvd, 40); // Volume identifier
    writeBothEndian32(jvd, 80, totalSectors);
    // Escape Sequences for UCS-2 Level 3 Joliet: 25 2F 45 ("%/E")
    jvd[88] = 0x25; jvd[89] = 0x2F; jvd[90] = 0x45;
    writeBothEndian16(jvd, 120, 1);
    writeBothEndian16(jvd, 124, 1);
    writeBothEndian16(jvd, 128, SECTOR_SIZE);
    writeBothEndian32(jvd, 132, 10);
    jvd.writeUInt32LE(23, 140); // Type L path table
    jvd.writeUInt32BE(24, 148); // Type M path table

    const jolietRootDirRec = createDirectoryRecord(true, '\x00', jolietDirLbas.get(''), SECTOR_SIZE, true);
    jolietRootDirRec.copy(jvd, 156);
    jvd[881] = 0x01;
    writeSectorAt(jvd, JOLIET_VD_LBA);

    // ==========================================
    // SECTOR 18: El Torito Boot Volume Descriptor
    // ==========================================
    const bvd = Buffer.alloc(SECTOR_SIZE, 0);
    bvd[0] = 0x00; // Type 0: Boot Record
    bvd.write("CD001", 1, 'ascii');
    bvd[6] = 0x01; // Version
    bvd.write("EL TORITO SPECIFICATION", 7, 'ascii');
    bvd.writeUInt32LE(BOOT_CATALOG_LBA, 0x47); // Pointer to Boot Catalog
    writeSectorAt(bvd, BOOT_VD_LBA);

    // ==========================================
    // SECTOR 19: Volume Descriptor Set Terminator
    // ==========================================
    const term = Buffer.alloc(SECTOR_SIZE, 0);
    term[0] = 0xFF; // Terminator
    term.write("CD001", 1, 'ascii');
    term[6] = 0x01;
    writeSectorAt(term, TERM_VD_LBA);

    // ==========================================
    // SECTOR 20: El Torito Boot Catalog
    // ==========================================
    const cat = Buffer.alloc(SECTOR_SIZE, 0);
    // Validation Entry (32 bytes)
    cat[0] = 0x01; // Header ID
    cat[1] = 0x00; // Platform ID: 0 = x86
    cat.write("MiniXP ISOLINUX Boot", 4, 'ascii');
    cat[30] = 0x55;
    cat[31] = 0xAA;
    // Calculate checksum for Validation Entry: sum of 16 words must be 0 mod 65536
    let sum16 = 0;
    for (let i = 0; i < 32; i += 2) {
        if (i !== 28) { // Skip checksum field itself
            sum16 += cat.readUInt16LE(i);
        }
    }
    const chk = (65536 - (sum16 % 65536)) % 65536;
    cat.writeUInt16LE(chk, 28);

    // Initial / Default Entry (32 bytes at offset 32)
    cat[32] = 0x88; // Bootable
    cat[33] = 0x00; // Media type: 0 = No Emulation
    cat.writeUInt16LE(0x0000, 34); // Load segment (0 = default 0x07C0)
    cat[36] = 0x00; // System type
    cat.writeUInt16LE(4, 38); // Sector count: 4 virtual sectors (4 * 512 = 2048 bytes = 1 CD sector)
    cat.writeUInt32LE(isolinuxBinEntry.lba, 40); // Load RBA: LBA of ISOLINUX.BIN
    writeSectorAt(cat, BOOT_CATALOG_LBA);

    // ==========================================
    // SECTOR 25.. Directory Contents (ISO & Joliet)
    // ==========================================
    for (const d of dirsList) {
        const info = dirMap.get(d);
        const isoBuf = Buffer.alloc(SECTOR_SIZE, 0);
        const jolietBuf = Buffer.alloc(SECTOR_SIZE, 0);

        let isoOffset = 0;
        let jolietOffset = 0;

        // '.' record
        const parentPath = path.dirname(d) === '.' ? '' : path.dirname(d).replace(/\\/g, '/');
        const selfIsoLba = isoDirLbas.get(d);
        const selfJolietLba = jolietDirLbas.get(d);
        const parentIsoLba = isoDirLbas.get(parentPath);
        const parentJolietLba = jolietDirLbas.get(parentPath);

        const isoSelf = createDirectoryRecord(false, '\x00', selfIsoLba, SECTOR_SIZE, true);
        isoSelf.copy(isoBuf, isoOffset);
        isoOffset += isoSelf.length;

        const jolietSelf = createDirectoryRecord(true, '\x00', selfJolietLba, SECTOR_SIZE, true);
        jolietSelf.copy(jolietBuf, jolietOffset);
        jolietOffset += jolietSelf.length;

        // '..' record
        const isoParent = createDirectoryRecord(false, '\x01', parentIsoLba, SECTOR_SIZE, true);
        isoParent.copy(isoBuf, isoOffset);
        isoOffset += isoParent.length;

        const jolietParent = createDirectoryRecord(true, '\x01', parentJolietLba, SECTOR_SIZE, true);
        jolietParent.copy(jolietBuf, jolietOffset);
        jolietOffset += jolietParent.length;

        // Children entries
        for (const child of info.children) {
            const childLba = child.isDir ? isoDirLbas.get(child.relPath) : fileLbas.get(child.relPath);
            const childJolietLba = child.isDir ? jolietDirLbas.get(child.relPath) : fileLbas.get(child.relPath);
            const childSize = child.isDir ? SECTOR_SIZE : child.size;

            const recIso = createDirectoryRecord(false, child.name, childLba, childSize, child.isDir);
            recIso.copy(isoBuf, isoOffset);
            isoOffset += recIso.length;

            const recJoliet = createDirectoryRecord(true, child.name, childJolietLba, childSize, child.isDir);
            recJoliet.copy(jolietBuf, jolietOffset);
            jolietOffset += recJoliet.length;
        }

        writeSectorAt(isoBuf, isoDirLbas.get(d));
        writeSectorAt(jolietBuf, jolietDirLbas.get(d));
    }

    // ==========================================
    // Archivos Data & ISOLINUX Boot-Info-Table Patch
    // ==========================================
    for (const e of entries) {
        if (!e.isDir) {
            const lba = fileLbas.get(e.relPath);
            const content = fs.readFileSync(e.fullPath);

            if (e.relPath.toLowerCase() === 'hbcd/isolinux.bin' || e.relPath.toLowerCase() === 'isolinux.bin') {
                // Patch ISOLINUX boot-info-table at offset 8
                const patched = Buffer.from(content);
                patched.writeUInt32LE(PVD_LBA, 8); // PVD LBA (16)
                patched.writeUInt32LE(lba, 12);     // Boot file LBA
                patched.writeUInt32LE(content.length, 16); // Boot file length
                // Clear checksum field and 40-byte reserved
                patched.fill(0, 20, 64);
                const checksum = calculateIsolinuxChecksum(patched);
                patched.writeUInt32LE(checksum, 20); // Checksum

                console.log(`[+] Parcheado ISOLINUX Boot Info Table: PVD=${PVD_LBA}, BootFileLBA=${lba}, Checksum=0x${checksum.toString(16)}`);
                fs.writeSync(fd, patched, 0, patched.length, lba * SECTOR_SIZE);
            } else {
                fs.writeSync(fd, content, 0, content.length, lba * SECTOR_SIZE);
            }
        }
    }

    fs.closeSync(fd);
    const finalSize = fs.statSync(outputIsoPath).size;
    console.log("==================================================================");
    console.log(`[OK] ISO generada exitosamente: ${outputIsoPath} (${(finalSize / (1024 * 1024)).toFixed(2)} MB)`);
    console.log("==================================================================");
}

const baseDir = path.resolve(__dirname, '..');
const src = process.argv[2] || path.join(baseDir, 'MiniXp');
const out = process.argv[3] || path.join(baseDir, 'Packaging', 'minixp-wasm', 'minixp.iso');

buildIso(src, out);
