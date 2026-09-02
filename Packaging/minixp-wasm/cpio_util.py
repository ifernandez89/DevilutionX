import io, gzip, struct, time

def create_cpio_archive(files_dict):
    """
    Crea un archivo CPIO en formato newc (070701) compatible con el kernel de Linux initramfs.
    files_dict: dict de { 'path/to/file': (bytes_content, mode) }
    """
    out = io.BytesIO()
    ino = 100
    
    for path, (content, mode) in files_dict.items():
        ino += 1
        name_bytes = path.encode('utf-8') + b'\x00'
        name_len = len(name_bytes)
        file_len = len(content)
        
        # cpio newc header: 110 bytes ASCII hex
        # magic: "070701" (6)
        # ino (8), mode (8), uid (8), gid (8), nlink (8), mtime (8), filesize (8)
        # devmajor (8), devminor (8), rdevmajor (8), rdevminor (8), namesize (8), check (8)
        header = f"070701{ino:08X}{mode:08X}{1001:08X}{50:08X}{1:08X}{int(time.time()):08X}{file_len:08X}{3:08X}{1:08X}{0:08X}{0:08X}{name_len:08X}{0:08X}".encode('ascii')
        
        out.write(header)
        out.write(name_bytes)
        # Pad header+name to 4-byte boundary
        pad1 = (4 - ((110 + name_len) % 4)) % 4
        out.write(b'\x00' * pad1)
        
        # Content
        out.write(content)
        # Pad content to 4-byte boundary
        pad2 = (4 - (file_len % 4)) % 4
        out.write(b'\x00' * pad2)
        
    # Trailer
    trailer_name = b'TRAILER!!!\x00'
    trailer_hdr = f"070701{0:08X}{0:08X}{0:08X}{0:08X}{1:08X}{0:08X}{0:08X}{0:08X}{0:08X}{0:08X}{0:08X}{len(trailer_name):08X}{0:08X}".encode('ascii')
    out.write(trailer_hdr)
    out.write(trailer_name)
    pad_trailer = (4 - ((110 + len(trailer_name)) % 4)) % 4
    out.write(b'\x00' * pad_trailer)
    
    return out.getvalue()

# Test
test_files = {
    'home/tc/Desktop/test.txt': (b'Hello Retro PC!\n', 0o100644),
    'home/tc/Desktop': (b'', 0o040755),
}
cpio_data = create_cpio_archive(test_files)
print(f"CPIO test generado: {len(cpio_data)} bytes, magic={cpio_data[:6]}")
