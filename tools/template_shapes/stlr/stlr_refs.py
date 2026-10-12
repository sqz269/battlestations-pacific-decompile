"""Capstone pass over each stl_probable body: absolute refs by kind and body size (index datarefs are incomplete)."""
import json, os, sqlite3, struct, bisect, collections
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86 import X86_OP_MEM, X86_OP_IMM
root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))  # repo root
pe = pefile.PE(json.load(open(r'J:\PROG\battlestations-pacific-decompile\exports\bsp\disk_binary.json'))['path'], fast_load=True)
img = pe.get_memory_mapped_image()
B = 0x400000
db = sqlite3.connect(os.path.join(root, 'local', 'bsp_index.sqlite'))
starts = sorted(r[0] for r in db.execute('select address from functions'))
md = Cs(CS_ARCH_X86, CS_MODE_32)
md.detail = True
EMPTY = 0xce3a0c


def kind(ref):
    if ref >= 0xe08000 and ref < 0x10a0000:
        return 'data'
    if 0xce2000 <= ref < 0xce2600:
        return 'iat'
    if 0xce2600 <= ref < 0xe08000:
        bs = img[ref - B:ref - B + 64]
        n = 0
        while n < len(bs) and 32 <= bs[n] < 127:
            n += 1
        if n >= 4 and n < len(bs) and bs[n] == 0:
            return 'str'
        v = struct.unpack_from('<I', img, ref - B)[0]
        if 0x401000 <= v < 0xce2000:
            return 'vptr'
        return 'const'
    return None


ALL = collections.Counter()
d = json.load(open(os.path.join(root, 'local', 'stlr_feat.json')))
for x in d:
    a = int(x['a'], 16)
    i = bisect.bisect_right(starts, a)
    end = starts[i] if i < len(starts) else a + 0x1000
    end = min(end, a + 0x4000)
    code = img[a - B:end - B]
    c = collections.Counter()
    x['rl'] = []
    refs = set()
    for ins in md.disasm(code, a):
        for op in ins.operands:
            v = None
            if op.type == X86_OP_MEM and op.mem.base == 0 and op.mem.disp:
                v = op.mem.disp & 0xffffffff
            elif op.type == X86_OP_IMM and ins.mnemonic not in ('call', 'jmp') and not ins.mnemonic.startswith('j'):
                v = op.imm & 0xffffffff
            if v and v != EMPTY and v not in refs:
                k = kind(v)
                if k:
                    ALL[(k, v)] += 1
                    refs.add(v)
                    c[k] += 1
                    x.setdefault('rl', []).append(k[0] + hex(v)[2:])
    x['size'] = end - a
    x['ck'] = dict(c)
json.dump(d, open(os.path.join(root, 'local', 'stlr_feat.json'), 'w'), indent=0)
print([(k, hex(v), n) for (k, v), n in ALL.most_common(20)])
