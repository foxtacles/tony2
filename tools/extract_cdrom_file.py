#!/usr/bin/env python3
"""Extract a file from an ISO9660 CD image (2048-byte .iso or raw 2352-byte .bin).

Used by CI to pull the original TONY2.EXE out of the game CD image without
needing bchunk/p7zip. Only what a Mode 1 single-track PC data CD needs:
no Joliet, no Rock Ridge, no interleaving.

Usage: extract_cdrom_file.py <image> <filename> <output>
"""

import struct
import sys


class CdImage:
    """Sector-addressed reader for .iso (2048) and raw .bin (2352) images."""

    def __init__(self, path):
        self.f = open(path, "rb")
        # Mode 1 raw sectors start with a 12-byte sync pattern.
        sync = b"\x00" + b"\xff" * 10 + b"\x00"
        self.raw = self.f.read(12) == sync
        self.sector_size = 2352 if self.raw else 2048
        self.data_offset = 16 if self.raw else 0

    def sector(self, lba, count=1):
        out = bytearray()
        for i in range(lba, lba + count):
            self.f.seek(i * self.sector_size + self.data_offset)
            out += self.f.read(2048)
        return bytes(out)

    def read_extent(self, lba, size):
        sectors = (size + 2047) // 2048
        return self.sector(lba, sectors)[:size]


def find_file(image, name):
    """Walk the ISO9660 directory tree for `name`; returns (lba, size)."""
    pvd = image.sector(16)
    if pvd[0:6] != b"\x01CD001":
        raise SystemExit("primary volume descriptor not found (not an ISO9660 image?)")
    root_lba = struct.unpack_from("<I", pvd, 156 + 2)[0]
    root_size = struct.unpack_from("<I", pvd, 156 + 10)[0]

    todo = [(root_lba, root_size)]
    name = name.upper()
    while todo:
        lba, size = todo.pop()
        directory = image.read_extent(lba, size)
        pos = 0
        while pos < len(directory):
            rec_len = directory[pos]
            if rec_len == 0:
                # Rest of this sector is padding; jump to the next one.
                pos = (pos // 2048 + 1) * 2048
                continue
            rec = directory[pos : pos + rec_len]
            ent_lba = struct.unpack_from("<I", rec, 2)[0]
            ent_size = struct.unpack_from("<I", rec, 10)[0]
            flags = rec[25]
            name_len = rec[32]
            ent_name = rec[33 : 33 + name_len].decode("ascii", "replace")
            if ent_name not in ("\x00", "\x01"):
                if flags & 0x02:
                    todo.append((ent_lba, ent_size))
                elif ent_name.split(";")[0].upper() == name:
                    return ent_lba, ent_size
            pos += rec_len
    raise SystemExit(f"{name} not found in image")


def main():
    if len(sys.argv) != 4:
        raise SystemExit(__doc__)
    image_path, name, out_path = sys.argv[1:]
    image = CdImage(image_path)
    lba, size = find_file(image, name)
    with open(out_path, "wb") as f:
        f.write(image.read_extent(lba, size))
    kind = "raw/2352" if image.raw else "iso/2048"
    print(f"extracted {name} ({size} bytes) from {kind} image -> {out_path}")


if __name__ == "__main__":
    main()
