/**
 * build_psp_iso.js
 * Empaquetador universal de imágenes ISO-9660 para juegos y homebrew de Sony PSP.
 * Toma una carpeta de origen (ej. G:\ o una carpeta con PSP_GAME) y genera un archivo .ISO estándar.
 * 
 * Uso:
 *   node tools/build_psp_iso.js [directorio_origen] [archivo_destino.iso]
 * 
 * Ejemplo:
 *   node tools/build_psp_iso.js G:\ iron_man_psp.iso
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
    buf[6] = 0; // GMT offset
    return buf;
}

class DirectoryNode {
    constructor(name, parent = null) {
        this.name = name;
        this.parent = parent || this;
        this.subdirs = new Map();
        this.files = [];
        this.lba = 0;
        this.sectorLength = 0;
        this.dirIndex = 1;
    }

    addFile(isoName, localPath, size) {
        this.files.push({ isoName, localPath, size, lba: 0 });
    }

    getOrAddSubdir(name) {
        const key = name.toUpperCase();
        if (!this.subdirs.has(key)) {
            const child = new DirectoryNode(key, this);
            this.subdirs.set(key, child);
        }
        return this.subdirs.get(key);
    }
}

function scanTree(sourceDir) {
    const root = new DirectoryNode('');

    function walk(currentDir, currentNode) {
        const entries = fs.readdirSync(currentDir, { withFileTypes: true });
        for (const ent of entries) {
            const fullPath = path.join(currentDir, ent.name);
            if (ent.isDirectory()) {
                const sub = currentNode.getOrAddSubdir(ent.name);
                walk(fullPath, sub);
            } else if (ent.isFile()) {
                const stat = fs.statSync(fullPath);
                currentNode.addFile(ent.name.toUpperCase(), fullPath, stat.size);
            }
        }
    }

    walk(sourceDir, root);
    return root;
}

function flattenDirectories(root) {
    const list = [];
    let idx = 1;

    function traverse(node) {
        node.dirIndex = idx++;
        list.push(node);
        const sortedSubdirs = Array.from(node.subdirs.values()).sort((a, b) => a.name.localeCompare(b.name));
        for (const sub of sortedSubdirs) {
            traverse(sub);
        }
    }

    traverse(root);
    return list;
}

function calculateDirRecordSize(nameLen) {
    const base = 33 + nameLen;
    return (base % 2 === 1) ? base + 1 : base;
}

function calculateDirectorySectors(node) {
    let currentSectorBytes = 0;
    let sectors = 1;

    function addRecord(nameLen) {
        const recSize = calculateDirRecordSize(nameLen);
        if (currentSectorBytes + recSize > SECTOR_SIZE) {
            sectors++;
            currentSectorBytes = recSize;
        } else {
            currentSectorBytes += recSize;
        }
    }

    addRecord(1); // '.'
    addRecord(1); // '..'

    for (const [name, sub] of node.subdirs.entries()) {
        addRecord(name.length);
    }

    for (const f of node.files) {
        const fileId = f.isoName.includes(';') ? f.isoName : `${f.isoName};1`;
        addRecord(fileId.length);
    }

    return sectors;
}

function buildDirectorySectorBuffers(node) {
    const sectors = [];
    let currentBuf = Buffer.alloc(SECTOR_SIZE, 0x00);
    let offset = 0;

    function writeRecord(recLba, recSize, flags, nameStr, isSpecial = 0) {
        const nameLen = isSpecial ? 1 : nameStr.length;
        const totalSize = calculateDirRecordSize(nameLen);

        if (offset + totalSize > SECTOR_SIZE) {
            sectors.push(currentBuf);
            currentBuf = Buffer.alloc(SECTOR_SIZE, 0x00);
            offset = 0;
        }

        const rec = Buffer.alloc(totalSize, 0x00);
        rec[0] = totalSize;
        rec[1] = 0; // Extended attribute
        writeBothEndian32(rec, 2, recLba);
        writeBothEndian32(rec, 10, recSize);
        formatDirDate().copy(rec, 18);
        rec[25] = flags;
        rec[26] = 0; // File unit size
        rec[27] = 0; // Interleave gap
        writeBothEndian16(rec, 28, 1); // Volume sequence number
        rec[32] = nameLen;

        if (isSpecial === 1) {
            rec[33] = 0x00; // '.'
        } else if (isSpecial === 2) {
            rec[33] = 0x01; // '..'
        } else {
            rec.write(nameStr, 33, 'ascii');
        }

        rec.copy(currentBuf, offset);
        offset += totalSize;
    }

    // '.'
    writeRecord(node.lba, node.sectorLength * SECTOR_SIZE, 0x02, '', 1);
    // '..'
    writeRecord(node.parent.lba, node.parent.sectorLength * SECTOR_SIZE, 0x02, '', 2);

    // Subdirectorios
    const sortedSubdirs = Array.from(node.subdirs.values()).sort((a, b) => a.name.localeCompare(b.name));
    for (const sub of sortedSubdirs) {
        writeRecord(sub.lba, sub.sectorLength * SECTOR_SIZE, 0x02, sub.name);
    }

    // Archivos
    const sortedFiles = [...node.files].sort((a, b) => a.isoName.localeCompare(b.isoName));
    for (const f of sortedFiles) {
        const fileId = f.isoName.includes(';') ? f.isoName : `${f.isoName};1`;
        writeRecord(f.lba, f.size, 0x00, fileId);
    }

    sectors.push(currentBuf);
    return sectors;
}

function createPspIso(sourceDir, outputIsoPath, volumeName = 'PSP_GAME') {
    sourceDir = path.resolve(sourceDir);
    outputIsoPath = path.resolve(outputIsoPath);

    console.log('='.repeat(65));
    console.log('[*] Generador de ISO para Sony PSP (PPSSPP WASM)');
    console.log(`[*] Origen:  ${sourceDir}`);
    console.log(`[*] Destino: ${outputIsoPath}`);
    console.log(`[*] Etiqueta de Volumen: ${volumeName}`);
    console.log('='.repeat(65));

    if (!fs.existsSync(sourceDir)) {
        throw new Error(`El directorio de origen no existe: ${sourceDir}`);
    }

    const root = scanTree(sourceDir);
    const dirList = flattenDirectories(root);

    // 16 sectores iniciales reservados (0 - 15)
    // Sector 16: Primary Volume Descriptor (PVD)
    // Sector 17: Volume Descriptor Set Terminator (VDST)
    // Sector 18+: Tablas de rutas (Path Tables)
    // Sector X+: Bloques de directorios
    // Sector Y+: Archivos de datos

    let currentLba = 18;

    // Path Tables
    const pathTableLbaLE = currentLba;
    currentLba += 2; // Reservar espacio para Path Tables LE y BE

    // Directorios LBA
    for (const d of dirList) {
        d.lba = currentLba;
        d.sectorLength = calculateDirectorySectors(d);
        currentLba += d.sectorLength;
    }

    // Archivos LBA
    let totalFiles = 0;
    let totalBytes = 0;
    for (const d of dirList) {
        for (const f of d.files) {
            f.lba = currentLba;
            const fileSectors = Math.ceil(f.size / SECTOR_SIZE) || 1;
            currentLba += fileSectors;
            totalFiles++;
            totalBytes += f.size;
        }
    }

    const totalVolumeSectors = currentLba;
    console.log(`[*] Directorios: ${dirList.length} | Archivos: ${totalFiles} | Total: ${(totalBytes / 1024 / 1024).toFixed(2)} MB`);
    console.log(`[*] Total de sectores ISO: ${totalVolumeSectors} (~${(totalVolumeSectors * 2048 / 1024 / 1024).toFixed(2)} MB)`);

    // Crear el archivo de salida
    const outStream = fs.createWriteStream(outputIsoPath);

    // Escribir 16 sectores de ceros
    const zeroSector = Buffer.alloc(SECTOR_SIZE, 0x00);
    for (let i = 0; i < 16; i++) {
        outStream.write(zeroSector);
    }

    // PVD (Sector 16)
    const pvd = Buffer.alloc(SECTOR_SIZE, 0x00);
    pvd[0] = 0x01; // Type: PVD
    pvd.write('CD001', 1, 'ascii'); // Standard Identifier
    pvd[6] = 0x01; // Version
    pvd[7] = 0x00; // Unused
    encodeAsciiPadded('', 32).copy(pvd, 8); // System ID
    encodeAsciiPadded(volumeName, 32).copy(pvd, 40); // Volume ID
    // 72: Unused (8 bytes)
    writeBothEndian32(pvd, 80, totalVolumeSectors); // Volume Space Size
    // 88: Escape sequences (32 bytes)
    writeBothEndian16(pvd, 120, 1); // Volume Set Size
    writeBothEndian16(pvd, 124, 1); // Volume Sequence Number
    writeBothEndian16(pvd, 128, SECTOR_SIZE); // Logical Block Size
    writeBothEndian32(pvd, 132, 2048); // Path Table Size (dummy)
    pvd.writeUInt32LE(pathTableLbaLE, 140); // Path Table L (LE)
    pvd.writeUInt32BE(pathTableLbaLE + 1, 148); // Path Table M (BE)

    // Root Directory Record en PVD (34 bytes)
    pvd[156] = 34; // Record Length
    pvd[157] = 0; // Extended Attribute
    writeBothEndian32(pvd, 158, root.lba); // LBA Root
    writeBothEndian32(pvd, 166, root.sectorLength * SECTOR_SIZE); // Size Root
    formatDirDate().copy(pvd, 174);
    pvd[181] = 0x02; // Flags: Directory
    writeBothEndian16(pvd, 184, 1); // Volume Sequence
    pvd[188] = 1; // Name Length
    pvd[189] = 0x00; // Root identifier

    encodeAsciiPadded(volumeName, 128).copy(pvd, 190); // Volume Set ID
    encodeAsciiPadded('PPSSPP_WASM', 128).copy(pvd, 318); // Publisher ID
    encodeAsciiPadded('DEVILUTIONX_RETRO_HUB', 128).copy(pvd, 446); // Data Preparer ID
    encodeAsciiPadded('BUILD_PSP_ISO', 128).copy(pvd, 574); // Application ID
    encodeAsciiPadded('', 37).copy(pvd, 702); // Copyright File
    encodeAsciiPadded('', 37).copy(pvd, 739); // Abstract File
    encodeAsciiPadded('', 37).copy(pvd, 776); // Bibliographic File
    formatIsoDate().copy(pvd, 813); // Creation Date
    formatIsoDate().copy(pvd, 830); // Modification Date
    Buffer.alloc(17, 0x30).copy(pvd, 847); pvd[863] = 0; // Expiration Date
    formatIsoDate().copy(pvd, 864); // Effective Date
    pvd[881] = 0x01; // File Structure Version
    outStream.write(pvd);

    // VDST (Sector 17)
    const vdst = Buffer.alloc(SECTOR_SIZE, 0x00);
    vdst[0] = 0xFF; // Type: Terminator
    vdst.write('CD001', 1, 'ascii');
    vdst[6] = 0x01;
    outStream.write(vdst);

    // Path Tables (Sectores 18 y 19)
    const ptLE = Buffer.alloc(SECTOR_SIZE, 0x00);
    const ptBE = Buffer.alloc(SECTOR_SIZE, 0x00);
    let ptOffsetLE = 0;
    let ptOffsetBE = 0;

    for (const d of dirList) {
        const nameLen = (d === root) ? 1 : d.name.length;
        const recLen = 8 + nameLen + (nameLen % 2 === 1 ? 1 : 0);

        // LE
        ptLE[ptOffsetLE] = nameLen;
        ptLE[ptOffsetLE + 1] = 0;
        ptLE.writeUInt32LE(d.lba, ptOffsetLE + 2);
        ptLE.writeUInt16LE(d.parent.dirIndex, ptOffsetLE + 6);
        if (d === root) ptLE[ptOffsetLE + 8] = 0x00;
        else ptLE.write(d.name, ptOffsetLE + 8, 'ascii');
        ptOffsetLE += recLen;

        // BE
        ptBE[ptOffsetBE] = nameLen;
        ptBE[ptOffsetBE + 1] = 0;
        ptBE.writeUInt32BE(d.lba, ptOffsetBE + 2);
        ptBE.writeUInt16BE(d.parent.dirIndex, ptOffsetBE + 6);
        if (d === root) ptBE[ptOffsetBE + 8] = 0x00;
        else ptBE.write(d.name, ptOffsetBE + 8, 'ascii');
        ptOffsetBE += recLen;
    }

    outStream.write(ptLE);
    outStream.write(ptBE);

    // Escribir Directorios
    for (const d of dirList) {
        const sectorBuffers = buildDirectorySectorBuffers(d);
        for (const buf of sectorBuffers) {
            outStream.write(buf);
        }
    }

    // Escribir Archivos
    for (const d of dirList) {
        for (const f of d.files) {
            const data = fs.readFileSync(f.localPath);
            outStream.write(data);
            const remainder = data.length % SECTOR_SIZE;
            if (remainder > 0) {
                const padding = Buffer.alloc(SECTOR_SIZE - remainder, 0x00);
                outStream.write(padding);
            }
        }
    }

    outStream.end();
    console.log(`[SUCCESS] ISO creada exitosamente: ${outputIsoPath}`);
}

// Ejecución CLI
if (require.main === module) {
    const args = process.argv.slice(2);
    const src = args[0] || 'G:\\';
    const dst = args[1] || 'iron_man_psp.iso';
    try {
        createPspIso(src, dst, 'IRON_MAN_PSP');
    } catch (e) {
        console.error('[ERROR]', e.message);
        process.exit(1);
    }
}

module.exports = { createPspIso };
