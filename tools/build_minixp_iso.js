/**
 * build_minixp_iso.js
 * Generador completo de ISO-9660 + Joliet con arranque El Torito (ISOLINUX / SeaBIOS)
 * para Mini Windows XP en WebAssembly (v86).
 * Incluye Path Tables estándar (L/M), réplicas de rutas de configuración de ISOLINUX
 * y parchado de Boot-Info-Table.
 */

const fs = require('fs');
const path = require('path');

const SECTOR_SIZE = 2048;

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

    // 1. Recolectar todos los archivos reales
    const rawEntries = [];
    function scan(currentDir, relPath = '') {
        const files = fs.readdirSync(currentDir);
        for (const file of files) {
            const fullPath = path.join(currentDir, file);
            const currentRel = relPath ? `${relPath}/${file}` : file;
            const stat = fs.statSync(fullPath);
            if (stat.isDirectory()) {
                scan(fullPath, currentRel);
            } else {
                rawEntries.push({
                    relPath: currentRel.replace(/\\/g, '/'),
                    fullPath: fullPath,
                    size: stat.size
                });
            }
        }
    }
    scan(sourceDir);

    // Encontrar archivos clave de arranque
    const isolinuxBin = rawEntries.find(e => e.relPath.toUpperCase() === 'HBCD/ISOLINUX.BIN' || e.relPath.toUpperCase() === 'ISOLINUX.BIN');
    const isolinuxCfg = rawEntries.find(e => e.relPath.toUpperCase() === 'HBCD/ISOLINUX.CFG' || e.relPath.toUpperCase() === 'ISOLINUX.CFG');
    const chainC32 = rawEntries.find(e => e.relPath.toUpperCase() === 'HBCD/BOOT/CHAIN.C32' || e.relPath.toUpperCase() === 'CHAIN.C32');
    const menuC32 = rawEntries.find(e => e.relPath.toUpperCase() === 'HBCD/BOOT/MENU.C32' || e.relPath.toUpperCase() === 'MENU.C32');

    if (!isolinuxBin) {
        throw new Error("No se encontró ISOLINUX.BIN");
    }

    // Configuración universal de ISOLINUX para auto-arranque
    const universalCfgContent = Buffer.from(
        "DEFAULT minixp\n" +
        "PROMPT 0\n" +
        "TIMEOUT 0\n\n" +
        "LABEL minixp\n" +
        "  MENU LABEL Mini Windows XP\n" +
        "  COM32 /HBCD/BOOT/CHAIN.C32\n" +
        "  APPEND ntldr=/HBCD/XP/XP.BIN\n",
        "utf8"
    );

    // Mapeo virtual de archivos en el árbol ISO
    // Aseguramos que ISOLINUX encuentre su archivo de configuración y dependencias en cualquier ruta estándar
    const allFiles = new Map();

    // Archivos de origen
    for (const e of rawEntries) {
        allFiles.set(e.relPath.toUpperCase(), {
            path: e.relPath,
            fullPath: e.fullPath,
            buffer: null,
            size: e.size
        });
    }

    // Réplicas esenciales para el cargador de ISOLINUX
    const mirrorFiles = [
        { isoPath: "ISOLINUX.CFG", buffer: universalCfgContent },
        { isoPath: "ISOLINUX/ISOLINUX.CFG", buffer: universalCfgContent },
        { isoPath: "BOOT/ISOLINUX/ISOLINUX.CFG", buffer: universalCfgContent },
        { isoPath: "HBCD/ISOLINUX.CFG", buffer: universalCfgContent },
    ];

    if (chainC32) {
        mirrorFiles.push({ isoPath: "CHAIN.C32", fullPath: chainC32.fullPath, size: chainC32.size });
        mirrorFiles.push({ isoPath: "ISOLINUX/CHAIN.C32", fullPath: chainC32.fullPath, size: chainC32.size });
        mirrorFiles.push({ isoPath: "BOOT/CHAIN.C32", fullPath: chainC32.fullPath, size: chainC32.size });
    }
    if (menuC32) {
        mirrorFiles.push({ isoPath: "MENU.C32", fullPath: menuC32.fullPath, size: menuC32.size });
        mirrorFiles.push({ isoPath: "ISOLINUX/MENU.C32", fullPath: menuC32.fullPath, size: menuC32.size });
        mirrorFiles.push({ isoPath: "BOOT/MENU.C32", fullPath: menuC32.fullPath, size: menuC32.size });
    }

    for (const m of mirrorFiles) {
        allFiles.set(m.isoPath.toUpperCase(), {
            path: m.isoPath,
            fullPath: m.fullPath || null,
            buffer: m.buffer || null,
            size: m.buffer ? m.buffer.length : m.size
        });
    }

    // Estructurar directorios
    const directories = new Set();
    directories.add(""); // root

    for (const [key, item] of allFiles.entries()) {
        const parts = item.path.split('/');
        let current = "";
        for (let i = 0; i < parts.length - 1; i++) {
            current = current ? `${current}/${parts[i]}` : parts[i];
            directories.add(current);
        }
    }

    const sortedDirs = Array.from(directories).sort((a, b) => {
        const depthA = a ? a.split('/').length : 0;
        const depthB = b ? b.split('/').length : 0;
        if (depthA !== depthB) return depthA - depthB;
        return a.localeCompare(b);
    });

    const dirMap = new Map();
    for (const d of sortedDirs) {
        dirMap.set(d, {
            path: d,
            name: d ? d.split('/').pop() : '',
            parent: d ? (d.includes('/') ? d.substring(0, d.lastIndexOf('/')) : '') : '',
            subdirs: [],
            files: []
        });
    }

    for (const d of sortedDirs) {
        if (d) {
            const p = dirMap.get(d).parent;
            dirMap.get(p).subdirs.push(d);
        }
    }

    for (const [key, item] of allFiles.entries()) {
        const p = item.path.includes('/') ? item.path.substring(0, item.path.lastIndexOf('/')) : '';
        dirMap.get(p).files.push(item);
    }

    // 2. Asignación de LBAs
    // Sector 16: PVD
    // Sector 17: JVD
    // Sector 18: Boot Record
    // Sector 19: Terminator
    // Sector 20: Boot Catalog
    // Sector 21: Path Table L (ISO)
    // Sector 22: Path Table M (ISO)
    // Sector 23: Path Table L (Joliet)
    // Sector 24: Path Table M (Joliet)
    // Sector 25..: Directory Blocks (ISO y Joliet)
    // Siguientes: Archivos de datos

    const PVD_LBA = 16;
    const JVD_LBA = 17;
    const BOOT_VD_LBA = 18;
    const TERM_VD_LBA = 19;
    const BOOT_CATALOG_LBA = 20;
    const PATH_L_ISO_LBA = 21;
    const PATH_M_ISO_LBA = 22;
    const PATH_L_JOL_LBA = 23;
    const PATH_M_JOL_LBA = 24;

    let currentLba = 25;

    const isoDirLbas = new Map();
    const jolietDirLbas = new Map();

    for (const d of sortedDirs) {
        isoDirLbas.set(d, currentLba++);
    }
    for (const d of sortedDirs) {
        jolietDirLbas.set(d, currentLba++);
    }

    // Asignar LBAs a archivos
    const fileLbas = new Map();
    let isolinuxBinLba = 0;
    let isolinuxBinSize = 0;

    for (const [key, item] of allFiles.entries()) {
        fileLbas.set(key, currentLba);
        const sectors = Math.ceil(item.size / SECTOR_SIZE) || 1;
        if (key === 'HBCD/ISOLINUX.BIN' || key === 'ISOLINUX.BIN') {
            isolinuxBinLba = currentLba;
            isolinuxBinSize = item.size;
        }
        currentLba += sectors;
    }

    const totalSectors = currentLba;
    console.log(`[+] Total Sectores ISO: ${totalSectors} (~${((totalSectors * 2048) / (1024 * 1024)).toFixed(2)} MB)`);
    console.log(`[+] ISOLINUX.BIN LBA: ${isolinuxBinLba}, Tamaño: ${isolinuxBinSize} bytes`);

    // 3. Construir Path Tables (ISO y Joliet)
    function buildPathTable(isJoliet, isBigEndian) {
        const buffers = [];
        for (let i = 0; i < sortedDirs.length; i++) {
            const dirPath = sortedDirs[i];
            const info = dirMap.get(dirPath);
            const lba = isJoliet ? jolietDirLbas.get(dirPath) : isoDirLbas.get(dirPath);
            const parentIdx = dirPath === '' ? 1 : (sortedDirs.indexOf(info.parent) + 1);

            let nameBuf;
            if (dirPath === '') {
                nameBuf = Buffer.from([0x00]);
            } else if (isJoliet) {
                nameBuf = Buffer.alloc(info.name.length * 2);
                for (let k = 0; k < info.name.length; k++) {
                    nameBuf.writeUInt16BE(info.name.charCodeAt(k), k * 2);
                }
            } else {
                nameBuf = Buffer.from(info.name.toUpperCase(), 'ascii');
            }

            const lenDi = nameBuf.length;
            const entryLen = 8 + lenDi + (lenDi % 2);
            const entry = Buffer.alloc(entryLen, 0);

            entry[0] = lenDi;
            entry[1] = 0; // EAR length
            if (isBigEndian) {
                entry.writeUInt32BE(lba, 2);
                entry.writeUInt16BE(parentIdx, 6);
            } else {
                entry.writeUInt32LE(lba, 2);
                entry.writeUInt16LE(parentIdx, 6);
            }
            nameBuf.copy(entry, 8);
            buffers.push(entry);
        }
        const fullBuf = Buffer.concat(buffers);
        const sectorBuf = Buffer.alloc(SECTOR_SIZE, 0);
        fullBuf.copy(sectorBuf, 0);
        return { buffer: sectorBuf, size: fullBuf.length };
    }

    const pathLIso = buildPathTable(false, false);
    const pathMIso = buildPathTable(false, true);
    const pathLJol = buildPathTable(true, false);
    const pathMJol = buildPathTable(true, true);

    // 4. Crear archivo de salida
    const fd = fs.openSync(outputIsoPath, 'w');
    const zeroSector = Buffer.alloc(SECTOR_SIZE, 0);

    function writeSectorAt(buf, lba) {
        fs.writeSync(fd, buf, 0, buf.length, lba * SECTOR_SIZE);
    }

    // Inicializar ISO
    fs.writeSync(fd, zeroSector, 0, SECTOR_SIZE, (totalSectors - 1) * SECTOR_SIZE);

    // PVD (Sector 16)
    const pvd = Buffer.alloc(SECTOR_SIZE, 0);
    pvd[0] = 0x01;
    pvd.write("CD001", 1, 'ascii');
    pvd[6] = 0x01;
    encodeAsciiPadded("MINIXP", 32).copy(pvd, 40);
    writeBothEndian32(pvd, 80, totalSectors);
    writeBothEndian16(pvd, 120, 1);
    writeBothEndian16(pvd, 124, 1);
    writeBothEndian16(pvd, 128, SECTOR_SIZE);
    writeBothEndian32(pvd, 132, pathLIso.size);
    pvd.writeUInt32LE(PATH_L_ISO_LBA, 140);
    pvd.writeUInt32BE(PATH_M_ISO_LBA, 148);

    const isoRootDirRec = createDirectoryRecord(false, '\x00', isoDirLbas.get(''), SECTOR_SIZE, true);
    isoRootDirRec.copy(pvd, 156);

    encodeAsciiPadded("MINIXP", 128).copy(pvd, 190);
    encodeAsciiPadded("DEVILUTIONX", 128).copy(pvd, 318);
    encodeAsciiPadded("DEVILUTIONX", 128).copy(pvd, 446);
    encodeAsciiPadded("ISOLINUX", 128).copy(pvd, 574);
    formatIsoDate().copy(pvd, 813);
    formatIsoDate().copy(pvd, 830);
    formatIsoDate().copy(pvd, 847);
    formatIsoDate().copy(pvd, 864);
    pvd[881] = 0x01;
    writeSectorAt(pvd, PVD_LBA);

    // JVD (Sector 17)
    const jvd = Buffer.alloc(SECTOR_SIZE, 0);
    jvd[0] = 0x02;
    jvd.write("CD001", 1, 'ascii');
    jvd[6] = 0x01;
    encodeAsciiPadded("MINIXP", 32).copy(jvd, 8);
    encodeJolietPadded("Mini Windows XP", 32).copy(jvd, 40);
    writeBothEndian32(jvd, 80, totalSectors);
    jvd[88] = 0x25; jvd[89] = 0x2F; jvd[90] = 0x45; // %/E
    writeBothEndian16(jvd, 120, 1);
    writeBothEndian16(jvd, 124, 1);
    writeBothEndian16(jvd, 128, SECTOR_SIZE);
    writeBothEndian32(jvd, 132, pathLJol.size);
    jvd.writeUInt32LE(PATH_L_JOL_LBA, 140);
    jvd.writeUInt32BE(PATH_M_JOL_LBA, 148);

    const jolietRootDirRec = createDirectoryRecord(true, '\x00', jolietDirLbas.get(''), SECTOR_SIZE, true);
    jolietRootDirRec.copy(jvd, 156);
    jvd[881] = 0x01;
    writeSectorAt(jvd, JVD_LBA);

    // Boot Record (Sector 18)
    const bvd = Buffer.alloc(SECTOR_SIZE, 0);
    bvd[0] = 0x00;
    bvd.write("CD001", 1, 'ascii');
    bvd[6] = 0x01;
    bvd.write("EL TORITO SPECIFICATION", 7, 'ascii');
    bvd.writeUInt32LE(BOOT_CATALOG_LBA, 0x47);
    writeSectorAt(bvd, BOOT_VD_LBA);

    // Terminator (Sector 19)
    const term = Buffer.alloc(SECTOR_SIZE, 0);
    term[0] = 0xFF;
    term.write("CD001", 1, 'ascii');
    term[6] = 0x01;
    writeSectorAt(term, TERM_VD_LBA);

    // Boot Catalog (Sector 20)
    const cat = Buffer.alloc(SECTOR_SIZE, 0);
    cat[0] = 0x01; // Header
    cat[1] = 0x00; // x86
    cat.write("ISOLINUX", 4, 'ascii');
    cat[30] = 0x55;
    cat[31] = 0xAA;
    let sum16 = 0;
    for (let i = 0; i < 32; i += 2) {
        if (i !== 28) sum16 += cat.readUInt16LE(i);
    }
    cat.writeUInt16LE((65536 - (sum16 % 65536)) % 65536, 28);

    cat[32] = 0x88; // Bootable
    cat[33] = 0x00; // No emulation
    cat.writeUInt16LE(0x0000, 34);
    cat[36] = 0x00;
    cat.writeUInt16LE(4, 38); // 4 virtual sectors = 2048 bytes
    cat.writeUInt32LE(isolinuxBinLba, 40);
    writeSectorAt(cat, BOOT_CATALOG_LBA);

    // Path Tables (Sectors 21..24)
    writeSectorAt(pathLIso.buffer, PATH_L_ISO_LBA);
    writeSectorAt(pathMIso.buffer, PATH_M_ISO_LBA);
    writeSectorAt(pathLJol.buffer, PATH_L_JOL_LBA);
    writeSectorAt(pathMJol.buffer, PATH_M_JOL_LBA);

    // Directorios (Sectors 25..)
    for (const d of sortedDirs) {
        const info = dirMap.get(d);
        const isoBuf = Buffer.alloc(SECTOR_SIZE, 0);
        const jolietBuf = Buffer.alloc(SECTOR_SIZE, 0);

        let isoOffset = 0;
        let jolietOffset = 0;

        const selfIsoLba = isoDirLbas.get(d);
        const selfJolietLba = jolietDirLbas.get(d);
        const parentIsoLba = d === '' ? selfIsoLba : isoDirLbas.get(info.parent);
        const parentJolietLba = d === '' ? selfJolietLba : jolietDirLbas.get(info.parent);

        // '.'
        const isoSelf = createDirectoryRecord(false, '\x00', selfIsoLba, SECTOR_SIZE, true);
        isoSelf.copy(isoBuf, isoOffset);
        isoOffset += isoSelf.length;

        const jolSelf = createDirectoryRecord(true, '\x00', selfJolietLba, SECTOR_SIZE, true);
        jolSelf.copy(jolietBuf, jolietOffset);
        jolietOffset += jolSelf.length;

        // '..'
        const isoPar = createDirectoryRecord(false, '\x01', parentIsoLba, SECTOR_SIZE, true);
        isoPar.copy(isoBuf, isoOffset);
        isoOffset += isoPar.length;

        const jolPar = createDirectoryRecord(true, '\x01', parentJolietLba, SECTOR_SIZE, true);
        jolPar.copy(jolietBuf, jolietOffset);
        jolietOffset += jolPar.length;

        // Subdirectorios
        for (const sub of info.subdirs) {
            const subInfo = dirMap.get(sub);
            const subIsoLba = isoDirLbas.get(sub);
            const subJolLba = jolietDirLbas.get(sub);

            const recIso = createDirectoryRecord(false, subInfo.name, subIsoLba, SECTOR_SIZE, true);
            recIso.copy(isoBuf, isoOffset);
            isoOffset += recIso.length;

            const recJol = createDirectoryRecord(true, subInfo.name, subJolLba, SECTOR_SIZE, true);
            recJol.copy(jolietBuf, jolietOffset);
            jolietOffset += recJol.length;
        }

        // Archivos
        for (const item of info.files) {
            const key = item.path.toUpperCase();
            const fLba = fileLbas.get(key);
            const fileName = item.path.split('/').pop();

            const recIso = createDirectoryRecord(false, fileName, fLba, item.size, false);
            recIso.copy(isoBuf, isoOffset);
            isoOffset += recIso.length;

            const recJol = createDirectoryRecord(true, fileName, fLba, item.size, false);
            recJol.copy(jolietBuf, jolietOffset);
            jolietOffset += recJol.length;
        }

        writeSectorAt(isoBuf, selfIsoLba);
        writeSectorAt(jolietBuf, selfJolietLba);
    }

    // Escribir datos de archivos
    for (const [key, item] of allFiles.entries()) {
        const lba = fileLbas.get(key);
        let content = item.buffer;
        if (!content && item.fullPath) {
            content = fs.readFileSync(item.fullPath);
        }

        if (key === 'HBCD/ISOLINUX.BIN' || key === 'ISOLINUX.BIN') {
            const patched = Buffer.from(content);
            patched.writeUInt32LE(PVD_LBA, 8);
            patched.writeUInt32LE(lba, 12);
            patched.writeUInt32LE(content.length, 16);
            patched.fill(0, 20, 64);
            const checksum = calculateIsolinuxChecksum(patched);
            patched.writeUInt32LE(checksum, 20);

            console.log(`[+] Parcheado Boot-Info-Table para ${key}: PVD=16, LBA=${lba}, Size=${content.length}, Checksum=0x${checksum.toString(16)}`);
            fs.writeSync(fd, patched, 0, patched.length, lba * SECTOR_SIZE);
        } else {
            fs.writeSync(fd, content, 0, content.length, lba * SECTOR_SIZE);
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
