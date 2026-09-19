"""Report each callee's RET imm and its tail instructions (for the x87 return question).

  python local/calleefx.py <addr> [<addr> ...]

Disassembles linearly from the entry until a run of >=2 INT3 (MSVC inter-function
padding), collecting every `ret`. Prints the distinct RET imm values and the last
few instructions before the final ret so the caller can see whether a float is
left in ST0 (an FLD/FSTP-free tail with EAX set means an integer/EAX return).
"""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import capstone
from x87trace import load_image, off_of


def main(argv):
    data, secs = load_image()
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    for spec in argv:
        addr = int(spec, 16)
        o = off_of(secs, addr)
        rets = []
        tail = []
        run = []
        int3 = 0
        for ins in md.disasm(data[o:o + 0x4000], addr):
            if ins.mnemonic == "int3":
                int3 += 1
                if int3 >= 2 and rets:
                    break
                continue
            int3 = 0
            run.append(ins)
            if ins.mnemonic in ("ret", "retn"):
                rets.append((ins.address, ins.op_str or "0"))
                tail = run[-6:]
        imms = sorted({r[1] for r in rets})
        print("%08x  rets=%d  imm=%s  span=%08x" % (addr, len(rets), ",".join(imms),
                                                    rets[-1][0] if rets else 0))
        for p in tail:
            print("    %08x  %-14s %s %s" % (p.address, p.bytes.hex(), p.mnemonic, p.op_str))


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
