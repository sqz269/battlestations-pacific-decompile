"""Verify three isolated timestamp bodies and emit ignored local reference bytes."""
import hashlib
import json
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86 import X86_OP_MEM, X86_OP_IMM, X86_REG_ESP, X86_REG_EAX, X86_REG_ESI
from ghidra_export import Client, ROOT, write


def main():
    config = json.loads((ROOT / 'config/target.json').read_text())
    client = Client(config)
    client.verify()
    pe = pefile.PE(config['binary'])
    ranges = {0x530890: 136, 0xbf7df0: 52, 0xbf7d40: 170}
    offsets, bodies, records = {}, {}, []
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
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True
    allowed = set('add sub mov push pop cmp sbb ret call jne jnz jge neg inc dec or '
                  'xor mul div shr rcr jb ja jbe jmp'.split())
    patches = []
    for address, body in bodies.items():
        expected = address
        for ins in md.disasm(body, address):
            if ins.address != expected or ins.mnemonic not in allowed:
                raise RuntimeError(f'Unexpected instruction at {ins.address:08x}: {ins.mnemonic}')
            for operand in ins.operands:
                if operand.type == X86_OP_MEM:
                    valid_bases = (X86_REG_ESP, X86_REG_EAX, X86_REG_ESI) if address == 0x530890 else (X86_REG_ESP,)
                    if operand.mem.index or operand.mem.base not in valid_bases:
                        raise RuntimeError('Unexpected memory reference')
            if ins.mnemonic == 'call':
                if address != 0x530890 or ins.operands[0].type != X86_OP_IMM or ins.imm_size != 4:
                    raise RuntimeError('Unexpected call form')
                target = ins.operands[0].imm
                if (ins.address, target) not in ((0x5308e4, 0xbf7df0), (0x5308f1, 0xbf7d40)):
                    raise RuntimeError('Unexpected call target')
                patches.append((offsets[address] + ins.address - address + ins.imm_offset, offsets[target]))
            elif ins.mnemonic.startswith('j'):
                if ins.operands[0].type != X86_OP_IMM or not address <= ins.operands[0].imm < address + len(body):
                    raise RuntimeError('Branch leaves isolated body')
            expected = ins.address + ins.size
        if expected != address + len(body):
            raise RuntimeError('Incomplete disassembly')
    if len(patches) != 2:
        raise RuntimeError('Expected exactly two internal call relocations')
    header = '#pragma once\ninline constexpr unsigned char timestamp_reference_bytes[] = {'
    header += ','.join(map(str, payload)) + '};\n'
    header += 'struct TimestampReferencePatch { unsigned offset, target; };\n'
    header += 'inline constexpr TimestampReferencePatch timestamp_reference_patches[] = {'
    header += ','.join('{%d,%d}' % entry for entry in patches) + '};\n'
    header += f'inline constexpr unsigned timestamp_reference_entry = {offsets[0x530890]};\n'
    write(ROOT / 'local/timestamp_reference.hpp', header)
    write(ROOT / 'local/timestamp_reference_audit.json', dict(ranges=records, relocation_count=len(patches),
        scope='Isolated timestamp subtraction and its two arithmetic helpers; no game loading'))
    print(f'Audited {len(payload)} bytes, {len(patches)} relocations; local/timestamp_reference.hpp emitted')


if __name__ == '__main__':
    main()
