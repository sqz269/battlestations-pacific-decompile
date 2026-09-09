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
    # Isolate the font writer before its optional child-UI branch. Replace the
    # SEH setup/local allocation with the same 58h total stack displacement.
    font_base, font_end = 0xab990c, 0xab9c5e
    font_full = verified(0xab98f0, 1747)
    if hashlib.sha256(font_full).hexdigest() != '795b76de15e3a52d032bfca739ca1cffe9714a83d7d619fd6aae9302aa13d236':
        raise RuntimeError('Unexpected full font writer bytes')
    font_body = font_full[font_base - 0xab98f0:font_end - 0xab98f0]
    wrapper = bytes.fromhex('83ec4853555657')  # SUB ESP,48h; PUSH EBX,EBP,ESI,EDI
    restore = bytes.fromhex('5f5e5d5b83c448c22800')
    font_patches = []
    end = font_base
    for instruction in disassembler.disasm(font_body, font_base):
        if instruction.mnemonic in ('call', 'ret', 'retf'):
            raise RuntimeError('Font prefix has an unexpected call/return')
        for operand in instruction.operands:
            if operand.type == X86_OP_MEM and not operand.mem.base and operand.mem.disp >= 0x400000:
                if operand.mem.disp not in (0xd7a24c, 0xcec380, 0xcef1b8, 0xe12fd4):
                    raise RuntimeError('Unexpected font absolute memory reference')
                font_patches.append((len(wrapper) + instruction.address - font_base + instruction.disp_offset, operand.mem.disp))
            if instruction.mnemonic.startswith('j'):
                if operand.type != X86_OP_IMM or not font_base <= operand.imm < font_end:
                    raise RuntimeError('Font branch leaves isolated prefix')
        end = instruction.address + instruction.size
    if end != font_end or len(font_patches) != 4:
        raise RuntimeError('Unexpected font prefix/relocation layout')
    for address, value in ((0xcec380, 960.0), (0xcef1b8, 720.0)):
        if verified(address, 8) != struct.pack('<d', value):
            raise RuntimeError('Font normalization constant changed')
    # 00e12fd4 is mutable: the fixture supplies a private explicit input.
    font_code = wrapper + font_body + restore
    header += 'inline constexpr unsigned char font_reference_bytes[] = {' + ','.join(map(str, font_code)) + '};\n'
    header += 'inline constexpr GuiReferencePatch font_reference_patches[] = {'
    header += ','.join('{' + str(offset) + ',' + str(target) + '}' for offset, target in font_patches) + '};\n'
    write(ROOT / 'local/gui_geometry_reference.hpp', header)
    write(ROOT / 'local/gui_reference_audit.json', dict(address=f'{base:08x}', length=length,
          sha256=hashlib.sha256(body).hexdigest(), disk_matches_ghidra=True,
          absolute_operands=patches, table_entries=entries))
    write(ROOT / 'local/font_reference_audit.json', dict(
          full_address='00ab98f0', full_length=len(font_full),
          full_sha256=hashlib.sha256(font_full).hexdigest(), disk_matches_ghidra=True,
          copied_start=f'{font_base:08x}', copied_end_exclusive=f'{font_end:08x}',
          copied_sha256=hashlib.sha256(font_body).hexdigest(),
          wrapper_hex=wrapper.hex(), restore_hex=restore.hex(),
          absolute_operands=font_patches, omitted='SEH registration and optional post-prefix child-UI path'))
    print('Verified GUI writer and isolated font prefix; emitted local reference header.')


if __name__ == '__main__':
    main()
