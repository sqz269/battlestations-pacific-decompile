"""Audit isolated cc13 leaf routines and emit the ignored local/cc13_leaf_reference.hpp.

Same pattern as verify_camera_reference.py: every code and constant byte range is read from the
executable on disk and compared with the saved Ghidra program, then packed into one payload.
Each absolute data operand or immediate is relocated to the payload copy of its range, and each
relative CALL to the payload copy of its callee. Anything else (an unlisted absolute address, a
call to an unlisted routine) refuses. Requires the Ghidra bridge and capstone. Never edits
Ghidra or the executable.

One substitution: LIBCRT_sqrt (00BF7030, `_CIsqrt`) dispatches into CRT code that is not copied;
calls to it are bound to the three bytes FSQRT; RET. Both take and return ST0, and for the
float-exact arguments these routines pass at 53-bit precision the CRT's SSE2 path (SQRTSD of
the double) gives the same value.
"""
import hashlib
import json
import struct
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86 import X86_OP_MEM, X86_OP_IMM
from ghidra_export import Client, ROOT, write

# Code ranges: address -> length through the final RET (asserted against the disassembly).
CODE = {
    0xbb9f40: 0x9f,   # CRC-32 table fill
    0xbba030: 0x3b,   # BSP_Crc32_Update
    0x8ef6a0: 0x30,   # hex digit value
    0x8ef6d0: 0x57,   # BIN token byte decode (calls 008EF6A0)
    0x8ee5f0: 0x74,   # scene text escape (static buffer 00F88C50)
    0x85d020: 0x319,  # segment / triangle intersection
    0x809e10: 0x10a,  # rocket flight time (calls 0042B2F0 and sqrt)
    0x42b2f0: 0x4d,   # BSP_Vector3_LengthFloatThreshold (callee of 00809E10)
    0x481c90: 0xa6,   # vector component min
    0x481d40: 0xa6,   # vector component max
    0x46f440: 0x107,  # point / line distance
    0x46fc80: 0x148,  # ray / plane intersection
    0x439bd0: 0x6a,   # quadratic roots
    0x73e780: 0x34,   # plane from normal and point
    0x73e7c0: 0x2e,   # plane signed distance
    0x569e40: 0x4b,   # linear remap
    0x59a840: 0x1e,   # world to map x
    0x59a860: 0x1e,   # world to map y
    0x709a60: 0x3b,   # vec3 add
    0x709bb0: 0x5f,   # vec3 add scaled
    0x78e2b0: 0xb3,   # movie camera angle windows contain
    0x78feb0: 0x1f2,  # movie camera angle windows clamp (calls 00438B10)
    0x438b10: 0x6e,   # BSP_Math_SubtractWrappedAngle (callee of 0078FEB0)
    # cc13-trivialslots: one trivial vtable slot per kind.
    0x626710: 0x6,    # ReturnInt: MOV EAX,4; RET (rewards screen id)
    0x636dd0: 0x3,    # ReturnByte: MOV AL,1; RET (scene-init occludes lower levels)
    0x67af30: 0x3,    # Empty: RET 8 (tactical sheet listbox selection changed)
    0x509d70: 0x8,    # Adjustor: ADD ECX,64h; JMP 006833F0 (stubbed)
    0x67b3f0: 0x5,    # TailJump: JMP 004F71D0 (stubbed)
    # cc13-fixture2: x87 / trig / sqrt / table leaves.
    0xb64470: 0x1c2,  # matrix from euler, scale, translation (FSIN/FCOS)
    0x467050: 0xd7,   # matrix rotation from euler (FSIN/FCOS)
    0x4e5c00: 0x11d,  # linearize rgb gamma 2.2 (FYL2X / F2XM1)
    0xb8c520: 0x173,  # color32 scale (FISTP)
    0xb65560: 0xf5,   # frustum planes from angles (FSIN/FCOS)
    0x452b00: 0x71,   # lens flare pow response (FYL2X / F2XM1)
    0xaed820: 0x6f,   # terrain float layer quantize to byte (chop FISTP)
    0x9064d0: 0x1d,   # score table map usage points (FISTP)
    0xb8b940: 0x32,   # truncate float to field +20h (chop FISTP qword)
    0xafb7f0: 0x247,  # ballistic max height (sqrt)
    0xafba40: 0x245,  # ballistic min height (sqrt)
    0x428090: 0x19f,  # segment intersects sphere (sqrt)
    0x941aa0: 0x17d,  # spawn ray / sphere (sqrt)
    0x9d7780: 0x1a5,  # oriented rect distance to point (sqrt)
    0x9d7930: 0x1b5,  # oriented rect 2 distance to point (sqrt)
    0x46f7b0: 0x184,  # project point onto plane (sqrt)
    0x46f550: 0x180,  # closest point on line (sqrt)
    0x46f6d0: 0xd7,   # distance along axis (sqrt)
    0x793730: 0xd0,   # vec3 scale to length (sqrt)
    0x793800: 0xb8,   # vec2 scale to length (sqrt)
    0x7bb4c0: 0xb1,   # triangle vertex distance sum (sqrt)
    0xa8b860: 0xc8,   # 2D line intersection
    0xb75580: 0x83,   # bezier
    0xb75520: 0x57,   # hermite basis
    0xbbf280: 0xe6,   # rgba lerp
    0x4f3da0: 0x138,  # 2D triangle contains point
    0xc37860: 0x125,  # 2D barycentric (register ABI)
    0xb478e0: 0xb,    # vertex element type size (table 00D61CC0)
    0xadfa90: 0x62,   # terrain u8 block set sample (chop FISTP)
    0xadc580: 0x66,   # terrain u16 block set sample (chop FISTP)
    0xb62dd0: 0x67,   # 2x2 invert in place
    0xb62a10: 0xb3,   # ortho off-centre LH
    # cc13-fixture3: Dyn and geometry leaves.
    0xc38810: 0x15c,  # cylinder shape support, double (slot 3; sqrt)
    0xc47aa0: 0x1f8,  # box shape support, float (slot 2)
    0xc38470: 0x13b,  # box shape support, double (slot 3)
    0xc412d0: 0x277,  # segment / box slab entry (register ABI)
    0xc313a0: 0x34e,  # polygon clip x at least (register ABI)
    0xc30d10: 0x349,  # polygon clip y at least (register ABI)
    0xc31060: 0x338,  # polygon clip x at most (register ABI)
    0xc309d0: 0x333,  # polygon clip y at most (register ABI)
    0xc34cf0: 0x272,  # expand node 3x4 matrices to 4x4 (register ABI)
    0xc37f60: 0xcd,   # body add force at point
    0xc43ea0: 0x15c,  # interpolate body transform
    # cc13-fixture5: game-logic float leaves (00438B10, a callee of 009B94E0, is above).
    0xbd4bf0: 0x131,  # matrix4x4 equals
    0x9fa510: 0x106,  # world edge overrun
    0x6fbce0: 0x106,  # elevator platform start travel
    0x78faf0: 0xdf,   # movie camera approach offset
    0x78fcf0: 0x76,   # movie camera ease in-out quad
    0x80e000: 0xed,   # unit AI order slot step
    0x4f3030: 0x59,   # segment lerp
    0x4439b0: 0x79,   # time of day hours
    0x6e2490: 0x4e,   # projectile shot speed
    0x681f40: 0x7a,   # game near world edge
    0x59c9b0: 0x7a,   # world point beyond map margin
    0x4f40c0: 0x68,   # is left turn
    0x4f2fd0: 0x57,   # circle contains point
    0x9b94e0: 0xa4,   # split-S roll step (calls 00438B10)
    0x9b9680: 0xb7,   # maneuver roll completed
    0x786030: 0xea,   # average peer quality
    0x815440: 0xd1,   # build unit order record
    0x7b9620: 0x98,   # plane altitude sink predicate
    0x727c30: 0xf9,   # index of readiest barrel
    # cc13-fixture8: small main-thread float leaves.
    0x809de0: 0x23, 0x7b5390: 0x6d, 0x791640: 0x71, 0x59ab10: 0xad, 0x790460: 0x61, 0x414130: 0x25,
    0x9c6220: 0x47, 0x88a0c0: 0x6d, 0x98a710: 0x3f, 0x638b20: 0x31, 0x42c450: 0x62, 0x7b5be0: 0x3a,
    0x6eb870: 0x53, 0x9b93f0: 0x21, 0xb0a3f0: 0x28, 0x6ea530: 0x15,
    # cc13-fixture4: main-thread math leaves (docs/CC13_FIXTURE_PASS_4.md).
    0x709ef0: 0x55a,  # flag cloth wind and springs (sqrt)
    0x70a450: 0x141,  # flag cloth integrate
    0x6f9ee0: 0x437,  # convex object box
    0xb917d0: 0x279,  # quadtree node clip range
    0xb94e90: 0x211,  # ocean FFT32 rows (sqrt)
    0xb950b0: 0x216,  # ocean FFT32 columns (sqrt)
    0xb6d440: 0x8b,   # matrix scale
    0xb6d630: 0x98,   # matrix add
    0xb6d6d0: 0x98,   # matrix subtract
    0xb65710: 0x1c1,  # plane set classify AABB
    0xb999f0: 0x14a,  # plane / segment intersection
    0x42c090: 0x114,  # random wander evaluate
    # cc13-fixture7 (Dyn supports/transforms and main-thread helpers).
    0xc48080: 0x72,   # sphere support (float)
    0xc38970: 0x46,   # sphere support (double)
    0xc377f0: 0x68,   # ccw 2D
    0xc33650: 0x6c,   # transform 3x4 to 4x4
    0xc336c0: 0x4d,   # transform from 4x4
    0xc37f10: 0x25,   # body inertia
    0x709b60: 0x43,   # vec3 add scaled
    0x6e0540: 0x47,   # vec3 add scaled (copy)
    0x59c920: 0x5b,   # world bounds center
    0x4b4850: 0x5b,   # map box center
    0x4193e0: 0x55,   # vector3 length squared
    0x427e30: 0x21,   # vector3 length squared
    0x4191d0: 0x3a,   # vector2 length squared
    0xbee140: 0x2f,   # scale vector4
    0x4e5d20: 0x29,   # rgba scale
    0xab6bd0: 0x58,   # text height
    0x7b8140: 0xa3,   # float window set sample and sum
    0x7ba450: 0x49,   # max3
    0xab0fe0: 0x51,   # gui colour equal
}
# Tail-jump targets replaced by MOV EAX,ECX; RET, so a fixture reads the ECX a thunk passes on.
TAIL_STUBS = (0x6833f0, 0x4f71d0)
TAIL_STUB = bytes([0x8b, 0xc1, 0xc3])
# Data ranges: address -> length. Bytes past a section's raw data are loader zero-fill.
DATA = {
    0xd0d098: 8, 0xd7a24c: 4, 0xd7a268: 8, 0xd7a280: 8, 0xce3820: 8, 0xd7a328: 8,
    0xce3828: 8, 0xce3d18: 8, 0xce3d28: 8,
    0xe08890: 4, 0xe08894: 4,      # map offsets (the fixture overwrites them)
    0x1090500: 0x400,              # CRC-32 table (BSS)
    0xf88c50: 0x400,               # scene text escape buffer (BSS)
    # cc13-fixture2 constants (floats and doubles read by the routines above).
    0xd7a208: 8, 0xce89d8: 8, 0xce4b48: 8, 0xcedd44: 8, 0xd7a260: 8, 0xce42d0: 8,
    0xce6628: 8, 0xd19660: 8, 0xd7a218: 8, 0xd7a308: 8, 0xd7a288: 8, 0xd7a2b0: 8,
    0xce6658: 8,
    0xd61cc0: 0x48,                # vertex element type sizes (18 dwords)
    # cc13-fixture3 constants: the double -1.0 at 00D7A250 and the double 1e-7 at 00D7A298.
    0xd7a250: 8, 0xd7a298: 8,
    # cc13-fixture5 constants (00D7A264 lies inside the 00D7A260 range above;
    # 00D045F0 is in the fixture8 block).
    0xce65d0: 8, 0xd7a358: 8, 0xce3938: 8, 0xce3d68: 8, 0xce3d70: 8,
    0xce4798: 8, 0xce4790: 8, 0xcf48a0: 8, 0xd03dd0: 8, 0xd20330: 8, 0xd7a258: 8,
    0xce3d40: 8, 0xce3dc0: 8,
    0xd7a238: 4, 0xce3800: 4, 0xce3804: 4, 0xce4788: 4, 0xce398c: 4, 0xce3958: 4,
    0xce7d7c: 4, 0xce38b8: 4, 0xce3854: 4,
    # cc13-fixture8: constants read by the routines above.
    0xd045f0: 8, 0xce3850: 4, 0xce3ae8: 4, 0xce3d78: 8, 0xd7a210: 8, 0xcf9058: 8,
    0xce7630: 8, 0xce38c8: 4, 0xce3d08: 4, 0xd7a278: 8,
    # cc13-fixture4 constants.
    0xd7a244: 4, 0xd7a248: 4, 0xd638e4: 4,  # and 00D7A210, listed by fixture8
    0xcef1b8: 8,                   # cc13-fixture7: the double 720.0 (00AB6BD0 divisor)
}
SQRT = 0xbf7030
SQRT_STUB = bytes([0xd9, 0xfa, 0xc3])


def main():
    config = json.loads((ROOT / 'config/target.json').read_text())
    client = Client(config)
    client.verify()
    pe = pefile.PE(config['binary'])
    base = pe.OPTIONAL_HEADER.ImageBase
    image_end = base + pe.OPTIONAL_HEADER.SizeOfImage

    def section_raw(address, length):
        rva = address - base
        section = next(s for s in pe.sections
                       if s.VirtualAddress <= rva < s.VirtualAddress + s.Misc_VirtualSize)
        return max(0, min(length, section.VirtualAddress + section.SizeOfRawData - rva))

    payload, offsets, records = bytearray(), {}, []
    for address, length in list(CODE.items()) + list(DATA.items()):
        raw = section_raw(address, length)
        original = pe.get_data(address - base, raw) if raw else b''
        if raw:
            saved = bytes.fromhex(client.get('read_memory', address=f'{address:08x}', length=raw)['hex'])
            if saved != original or len(original) != raw:
                raise RuntimeError(f'Disk/saved-program mismatch at {address:08x}')
        while len(payload) % 16:
            payload.append(0xcc)
        offsets[address] = len(payload)
        payload.extend(original + bytes(length - raw))
        records.append(dict(address=f'{address:08x}', length=length, disk_bytes=raw,
                            zero_fill=length - raw, sha256=hashlib.sha256(original).hexdigest()))
    while len(payload) % 16:
        payload.append(0xcc)
    offsets[SQRT] = len(payload)
    payload.extend(SQRT_STUB)
    for stub in TAIL_STUBS:
        while len(payload) % 16:
            payload.append(0xcc)
        offsets[stub] = len(payload)
        payload.extend(TAIL_STUB)

    def relocate(target):
        for start, length in DATA.items():
            if start <= target < start + length:
                return offsets[start] + target - start
        if target in CODE:
            return offsets[target]
        raise RuntimeError(f'Unlisted absolute address {target:08x}')

    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True
    patches = []
    for address, length in CODE.items():
        end = address
        here = offsets[address] - address
        tail = False
        for ins in md.disasm(bytes(payload[offsets[address]:offsets[address] + length]), address):
            if ins.mnemonic == 'jmp' and ins.operands[0].type == X86_OP_IMM \
                    and ins.operands[0].imm in TAIL_STUBS:
                patches.append((here + ins.address + ins.imm_offset, offsets[ins.operands[0].imm], True))
                tail = ins.address + ins.size == address + length
            elif ins.mnemonic == 'call' and ins.operands[0].type == X86_OP_IMM:
                target = ins.operands[0].imm
                if target != SQRT and target not in CODE:
                    raise RuntimeError(f'{address:08x}: call to unlisted {target:08x}')
                patches.append((here + ins.address + ins.imm_offset, offsets[target], True))
            elif (ins.group(1) or ins.group(7)) and ins.operands[0].type == X86_OP_IMM:
                if not address <= ins.operands[0].imm < address + length:
                    raise RuntimeError(f'{address:08x}: branch leaves the routine')
            else:
                for op in ins.operands:
                    if op.type == X86_OP_MEM and base <= op.mem.disp < image_end:
                        assert ins.disp_size == 4
                        patches.append((here + ins.address + ins.disp_offset, relocate(op.mem.disp), False))
                    elif op.type == X86_OP_IMM and base <= op.imm < image_end:
                        assert ins.imm_size == 4
                        patches.append((here + ins.address + ins.imm_offset, relocate(op.imm), False))
            end = ins.address + ins.size
        if end != address + length or not tail and payload[offsets[address] + length - 3] not in (0xc2, 0xc3) \
                and payload[offsets[address] + length - 1] != 0xc3:
            raise RuntimeError(f'{address:08x}: disassembly ends at {end:08x}, not at a RET')

    header = '// Generated by tools/verify_cc13_leaf_reference.py; ignored, local only.\n#pragma once\n'
    header += 'inline constexpr unsigned char cc13_leaf_bytes[] = {' + ','.join(map(str, payload)) + '};\n'
    header += 'struct Cc13LeafPatch { unsigned offset, target; bool relative; };\n'
    header += 'inline constexpr Cc13LeafPatch cc13_leaf_patches[] = {'
    header += ','.join('{%d,%d,%s}' % (a, b, str(c).lower()) for a, b, c in patches) + '};\n'
    for address in list(CODE) + list(DATA):
        header += f'inline constexpr unsigned cc13_leaf_{address:08x} = {offsets[address]};\n'
    write(ROOT / 'local/cc13_leaf_reference.hpp', header)
    write(ROOT / 'reports/cc13_leaf_reference_audit.json', json.dumps(
        dict(ranges=records, relocation_count=len(patches),
             substitution='00BF7030 LIBCRT_sqrt bound to FSQRT; RET',
             scope='Isolated leaf routines only; no game loading'), indent=2) + '\n')
    print(f'Audited {len(payload)} bytes, {len(patches)} relocations; local/cc13_leaf_reference.hpp emitted')


if __name__ == '__main__':
    main()
