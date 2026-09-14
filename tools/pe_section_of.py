"""Which PE section an address lives in, and whether it is file-backed.

A `.data` address past the section's raw size is zero-filled by the loader, so
reading it in Ghidra gives 0.0 and that says nothing about its runtime value.
`battlestationspacific.exe` has `.data va=00E08000 vsize=00297EDC rawsize=00010000`,
so everything past 00E18000 is in that category - including the plane rotation
factors at 00F872FC, whose static zeros nearly got published as "a disabled
tuning term". See docs/PLANE_CONTROL_RATE_LAW.md.

Usage:
    python tools/pe_section_of.py <exe> <addr> [<addr> ...]     addresses in hex

Read-only.
"""
import struct, sys
path = sys.argv[1]
want = [int(a, 16) for a in sys.argv[2:]]
data = open(path, 'rb').read()
pe = struct.unpack_from('<I', data, 0x3c)[0]
nsec = struct.unpack_from('<H', data, pe + 6)[0]
optsz = struct.unpack_from('<H', data, pe + 20)[0]
base = struct.unpack_from('<I', data, pe + 24 + 28)[0]
tbl = pe + 24 + optsz
print('imagebase %08x sections %d' % (base, nsec))
for i in range(nsec):
    o = tbl + i * 40
    name = data[o:o+8].rstrip(b'\0').decode('latin1')
    vsize, va, rawsize, rawptr = struct.unpack_from('<IIII', data, o + 8)
    print('%-9s va=%08x vsize=%08x rawsize=%08x' % (name, base + va, vsize, rawsize))
    for w in want:
        if base + va <= w < base + va + vsize:
            inraw = w - (base + va) < rawsize
            print('   -> %08x is in %s, %s' % (w, name, 'file-backed' if inraw else 'UNINITIALISED (bss-like)'))
