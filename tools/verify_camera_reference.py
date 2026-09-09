"""Audit five isolated native camera helpers and emit an ignored probe header."""
import hashlib
import json
import struct
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86 import X86_OP_MEM, X86_OP_IMM
from ghidra_export import Client, ROOT, write


def main():
    config = json.loads((ROOT / 'config/target.json').read_text())
    client = Client(config)
    client.verify()
    pe = pefile.PE(config['binary'])
    ranges = {0x413920: 874, 0xb63b30: 531, 0xb642f0: 176, 0x412e20: 19, 0xb6d4d0: 339,
              0xd7a208: 4, 0xd7a280: 8, 0xd7a24c: 4}
    offsets, records, bodies = {}, [], {}
    payload = bytearray()
    for address, length in ranges.items():
        original = pe.get_data(address - pe.OPTIONAL_HEADER.ImageBase, length)
        saved = bytes.fromhex(client.get('read_memory', address=f'{address:08x}', length=length)['hex'])
        if original != saved or len(original) != length:
            raise RuntimeError(f'Native bytes differ at {address:08x}')
        offsets[address] = len(payload)
        bodies[address] = original
        payload.extend(original)
        records.append(dict(address=f'{address:08x}', length=length,
                            sha256=hashlib.sha256(original).hexdigest(), disk_matches_ghidra=True))
    assert bodies[0xd7a208] == struct.pack('<I', 0x80000000)
    assert bodies[0xd7a280] == struct.pack('<d', .5)
    assert bodies[0xd7a24c] == struct.pack('<f', 1)
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True
    allowed = set('add sub mov fld fst fstp fmul fmulp fadd faddp fxch ret fdiv fdivp fdivr fdivrp '
                  'movaps movss pop push subss call fchs fld1 fsubr xorps fsincos'.split())
    patches = []
    for address in list(ranges)[:5]:
        end = address
        for ins in md.disasm(bodies[address], address):
            if ins.mnemonic not in allowed and ins.mnemonic != 'rep movsd':
                raise RuntimeError(f'Unexpected instruction {ins.mnemonic}')
            for operand in ins.operands:
                if operand.type == X86_OP_MEM and not operand.mem.base:
                    target = operand.mem.disp
                    if operand.mem.index or target not in (0xd7a208, 0xd7a280, 0xd7a24c):
                        raise RuntimeError('Unexpected absolute memory operand')
                    assert ins.disp_size == 4
                    patches.append((offsets[address] + ins.address - address + ins.disp_offset,
                                    offsets[target], False))
            if ins.mnemonic == 'call':
                assert address == 0xb642f0 and ins.operands[0].type == X86_OP_IMM
                assert ins.operands[0].imm == 0x412e20 and ins.imm_size == 4
                patches.append((offsets[address] + ins.address - address + ins.imm_offset,
                                offsets[0x412e20], True))
            end = ins.address + ins.size
        assert end == address + ranges[address]
    header = '#pragma once\ninline constexpr unsigned char camera_reference_bytes[] = {'
    header += ','.join(map(str, payload)) + '};\n'
    header += 'struct CameraReferencePatch { unsigned offset, target; bool relative; };\n'
    header += 'inline constexpr CameraReferencePatch camera_reference_patches[] = {'
    header += ','.join('{%d,%d,%s}' % (a,b,str(c).lower()) for a,b,c in patches) + '};\n'
    for address in list(ranges)[:5]:
        header += f'inline constexpr unsigned camera_reference_{address:08x} = {offsets[address]};\n'
    write(ROOT / 'local/camera_reference.hpp', header)
    write(ROOT / 'reports/camera_reference_audit.json', json.dumps(
        dict(ranges=records, relocation_count=len(patches), scope='Isolated helpers only; no game loading'), indent=2) + '\n')
    print(f'Audited {len(payload)} bytes, {len(patches)} relocations; local/camera_reference.hpp emitted')


if __name__ == '__main__':
    main()
