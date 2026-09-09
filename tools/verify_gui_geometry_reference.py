"""Verify and emit the optional isolated GUI reference into ignored local/.

Requires the existing Ghidra bridge and Python capstone. Never edits Ghidra or
the executable. The probe relocates only its private copy and jump table.
"""
import hashlib
import json
import struct
from pathlib import Path
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86 import X86_OP_MEM, X86_OP_IMM
from ghidra_export import Client, ROOT, pe_summary, write


def main():
    client = Client(json.loads((ROOT / 'config/target.json').read_text()))
    client.verify()
    binary = Path(client.config['binary'])
    summary, disk = pe_summary(binary), binary.read_bytes()
    base, length = 0xab1860, 0x688

    def verified(address, count):
        rva = address - int(summary['image_base'], 16)
        section = next(s for s in summary['sections'] if int(s['rva'], 16) <= rva
                       and rva + count <= int(s['rva'], 16) + s['raw_size'])
        offset = section['raw_offset'] + rva - int(section['rva'], 16)
        saved = bytes.fromhex(client.get('read_memory', address=f'{address:08x}', length=count)['hex'])
        if saved != disk[offset:offset + count]:
            raise RuntimeError(f'Disk/saved-image mismatch at {address:08x}')
        return saved

    body = verified(base, length)
    disassembler = Cs(CS_ARCH_X86, CS_MODE_32)
    disassembler.detail = True
    patches = []
    end = base
    for instruction in disassembler.disasm(body[:0x671], base):
        if instruction.mnemonic == 'call':
            raise RuntimeError('Reference has an unexpected call')
        for operand in instruction.operands:
            if operand.type == X86_OP_MEM and not operand.mem.base and operand.mem.disp >= 0x400000:
                if operand.mem.disp not in (0xd7a24c, 0xe12fd0, 0xab1ed4):
                    raise RuntimeError('Unexpected absolute memory reference')
                patches.append((instruction.address - base + instruction.disp_offset, operand.mem.disp))
            if operand.type == X86_OP_IMM and instruction.mnemonic.startswith('j'):
                if not base <= operand.imm < base + 0x671:
                    raise RuntimeError('Branch leaves isolated code')
        end = instruction.address + instruction.size
    if end != base + 0x671 or len(patches) != 7:
        raise RuntimeError('Unexpected native body/relocation layout')
    entries = struct.unpack_from('<5I', body, 0x674)
    if not all(base <= entry < base + 0x671 for entry in entries):
        raise RuntimeError('Jump table leaves isolated code')
    for address, value in ((0xd7a24c, 1.0), (0xe12fd0, 0.75)):
        if verified(address, 4) != struct.pack('<f', value):
            raise RuntimeError('Native constant changed')
    header = '// Verified isolated native GUI writer; generated locally.\n#pragma once\n'
    header += 'inline constexpr unsigned char gui_reference_bytes[] = {' + ','.join(map(str, body)) + '};\n'
    header += 'struct GuiReferencePatch { unsigned offset; unsigned target; };\n'
    header += 'inline constexpr GuiReferencePatch gui_reference_patches[] = {'
    header += ','.join('{' + str(offset) + ',' + str(target) + '}' for offset, target in patches) + '};\n'
    write(ROOT / 'local/gui_geometry_reference.hpp', header)
    write(ROOT / 'local/gui_reference_audit.json', dict(address=f'{base:08x}', length=length,
          sha256=hashlib.sha256(body).hexdigest(), disk_matches_ghidra=True,
          absolute_operands=patches, table_entries=entries))
    print('Verified GUI code, jump table and constants; emitted local reference header.')


if __name__ == '__main__':
    main()
